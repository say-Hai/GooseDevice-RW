#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>
#include <thread>
#include <mutex>
#include <unordered_map>
#include "Defines.h"
#include "UtilityClass.h"
#include "Serial.h"
#include "RFID.h"
#include "WEIGHT.h"
#include "Logger.h"
#include "WeightReckon.h"
using namespace std;
mutex mtx;
extern Logger weightLog;
extern Logger rfidLog;
vector<uint8_t> resetRFIDReader = {0xA0, 0x03, 0x01, 0x70, 0xEC};
vector<unsigned char> send_rfidCommand = { 0xA0, 0x0D, 0x01, 0x8A, 0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x03, 0x01, 0x00, 0x02, 0xBC};
vector<uint8_t> send_weightCommand = {0x01, 0x03, 0x00, 0x0B, 0x00, 0x08, 0x35, 0xCE};

void sqlStoreWeightThread(vector<WEIGHT> weight_vec);

void start_RFID_Thread() {
    unique_lock<mutex> lock(mtx);
    cout << "OpenRFID: ";
    unique_ptr<Serial> serRFID = make_unique<Serial>(RFIDCOM, CBR_115200);
    serRFID->serialConnection(); //连接串口
    unordered_map<pair<string, string>, vector<RFID>, pair_hash> epcBarKeyMap;
    lock.unlock();
    while (true) {
        if (serRFID->get_h_com() == INVALID_HANDLE_VALUE)
            serRFID->serialConnection();
        //1.原始数据
        vector<uint8_t> receiveData = serRFID->getComData(send_rfidCommand);
        //2.转成RFID数据
        vector<RFID> rfid_vec = dataToRFID(receiveData);
        //3.对RFID数据按照{epc, bar}进行分组
        sortRFIDData(rfid_vec, epcBarKeyMap);
        //4.遍历分组，找出已经离开设备的分组
        //    4.1.CurrTime - 最后一次读取到该鹅的Time >= OUT_NESTLE,判定鹅已经离开通道
        for (auto item = epcBarKeyMap.begin(); item != epcBarKeyMap.end();) {
            string curTime = getCurrentTime();
            string epc_endTime = item->second.back().startTime;
            string timeGap = calcuTimeGap(epc_endTime, curTime);
            if (timeGap >= OUT_NESTLE) {
                vector<RFID> temvec = {item->second.front(), item->second.back()};
                rfidLog.log(temvec.front().epc + ": " +
                            to_string(temvec.front().ant) + " -->  " + to_string(temvec.back().ant));
                for (auto i: item->second) {
                    rfidLog.log(i.epc + "    " + i.startTime + "     " + to_string(i.ant));
                }
                isEnterOrExit(temvec);

                item = epcBarKeyMap.erase(item);
            } else item++;
        }
    }
}

void start_Weight_Thread() {
    unique_lock<mutex> lock(mtx);
    cout << "OpenWeight: ";
    unique_ptr<Serial> serWeight = make_unique<Serial>(WEIGHTCOM, CBR_9600);
    serWeight->serialConnection(); //连接串口

    vector<WEIGHT>  weight_vec;
    vector<bool> hasOutPutInitWeight = {false, false, false, false};
    deque<double> initData(5);

    lock.unlock();
    while (true) {
        if (serWeight->get_h_com() == INVALID_HANDLE_VALUE)
            serWeight->serialConnection();
        //       1.获取串口原始数据
        string recv_weight = serWeight->read_weight_64();
        //处理重量值
        vector<double> w_vec = getWeight(recv_weight);
        initData.push_back(w_vec[0]);
        for(auto i_w_vec : w_vec) {
            int i_w = i_w_vec - initData.front();
            if (i_w < -50) {
                initData.push_back(i_w_vec);
                initData.pop_front();
                continue;
            }
            //如果不为负值，则体重已经完全降低到平稳，可用于计算
            if (i_w < 50) {
                initData.push_back(i_w_vec);
                if (initData.size() > 10) {
                    initData.pop_front();
                }
            }
            //目前还在通道，且体重数据未满，则加入到vec中
            if (i_w >= 100 && weight_vec.size() <= 70) {
                WEIGHT temp;
                string curTime = getCurrentTime();
                temp.weight = i_w;
                temp.startTime = curTime;
                temp.bar_col = "column_1";
                weight_vec.push_back(temp);
            } else if (i_w > 100 && weight_vec.size() > 70) {
            } else if (i_w < 100 && weight_vec.size() > 5) {
                //已经离开通道
                thread thread_weight_handle(sqlStoreWeightThread, weight_vec);
                thread_weight_handle.detach();
                weight_vec.clear();
                initData.clear();
                //计算时用的体重值
            }
        }
    }
}

void sqlStoreWeightThread(vector<WEIGHT> weight_vec) {
    unique_ptr<SQL_Conn> sql_instance = make_unique<SQL_Conn>();
    for (int i = 0; i < weight_vec.size(); i++) {
        string user_id = weight_vec[i].user_id;
        int weight = weight_vec[i].weight;
        string time = weight_vec[i].startTime;
        string column = weight_vec[i].bar_col;
        sql_instance->insert_originWeight(user_id, column, time, weight);
    }
}

int main() {
    thread rfid_thread(start_RFID_Thread);
    thread weight_thread(start_Weight_Thread);
    rfid_thread.join();
    weight_thread.join();
}
