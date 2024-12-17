#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>
#include <thread>
#include <unordered_map>
#include "Defines.h"
#include "UtilityClass.h"
#include "Serial.h"
#include "RFID.h"
#include "WEIGHT.h"
#include "Logger.h"
#include "WeightReckon.h"
using namespace std;
extern Logger weightLog;
extern Logger rfidLog;
extern Logger error;
vector<uint8_t> resetRFIDReader = {0xA0, 0x03, 0x01, 0x70, 0xEC};

vector<uint8_t> send_rfidCommand = {
    0xA0, 0x15, 0x01, 0x8A, 0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x03, 0x01,
    0x04, 0x01, 0x05, 0x01, 0x06, 0x01, 0x07, 0x01,
    0x00, 0x01, 0x9B
};
vector<uint8_t> send_weightCommand = {0x01, 0x03, 0x00, 0x0B, 0x00, 0x08, 0x35, 0xCE};

void sqlStoreWeightThread(vector<WEIGHT> weight_vec);

/**
 * @brief RFID线程处理数据
 */
void start_RFID_Thread() {
    try {
        cout << "OpenRFID: ";
        unique_ptr<Serial> serRFID;
        unordered_map<pair<string, string>, vector<RFID>, pair_hash> epcBarKeyMap;
        try {
            serRFID = make_unique<Serial>(RFIDCOM, CBR_115200);
            serRFID->serialConnection(); //连接串口
        } catch (const std::exception &e) {
            // 捕获并打印标准异常信息
            error.log(" 串口连接异常 serRFID: " + string(e.what()));
        }

        while (true) {
            vector<uint8_t> receiveData;
            vector<RFID> rfid_vec;
            try {
                //1.原始数据
                receiveData = serRFID->getComData(send_rfidCommand);
                //2.转成RFID数据
                rfid_vec = dataToRFID(receiveData);
                //3.对RFID数据按照{epc, bar}进行分组
                sortRFIDData(rfid_vec, epcBarKeyMap);
            } catch (const std::exception &e) {
                error.log("获取rfid数据异常 " + string(e.what()));
            }
            try {
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
            } catch (const std::exception &e) {
                error.log("rfid进出判断异常 " + string(e.what()));
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }catch (...) {
        // 捕获所有其他类型的异常
        std::cerr << "An unexpected error occurred" << std::endl;
    }
}

/**
 * @brief 称重线程处理数据
 */
void start_Weight_Thread() {
    try {
        cout << "OpenWeight: ";
        unique_ptr<Serial> serWeight;
        try {
            serWeight = make_unique<Serial>(WEIGHTCOM, CBR_9600);
            serWeight->serialConnection(); //连接串口
        } catch (const std::exception &e) {
            error.log("weight 串口打开错误" + string(e.what()));
        }

        vector<vector<WEIGHT> > weight_vec(4);
        vector<bool> hasOutPutInitWeight = {false, false, false, false};
        vector<deque<int> > initData(5);

        while (true) {
            vector<uint8_t> receiveData;
            vector<int> i_w_vec;
            try {
                //1.获取串口原始数据
                receiveData = serWeight->getComData(send_weightCommand, 21);
                //处理重量值
                i_w_vec = dataToWEIGHT(receiveData);
            } catch (const std::exception &e) {
                error.log("获取重量值失败 " + string(e.what()));
            }
            try {
                if (i_w_vec.size() < 4)
                    continue;
                //记录第一次测量的体重
                for (int i = 0; i < 4; i++) {
                    if (initData[i].size() == 0) {
                        initData[i].push_back(i_w_vec[i]);
                    }
                }

                //判断种鹅是否离开通道
                for (int i = 0; i < i_w_vec.size(); i++) {
                    int i_w = i_w_vec[i] - initData[i].front();
                    //如果为负值，则说明所取得初始值存在问题。跳过此次循环
                    if (i_w < -50) {
                        initData[i].push_back(i_w_vec[i]);
                        initData[i].pop_front();
                        continue;
                    }
                    //如果不为负值，则体重已经完全降低到平稳，可用于计算
                    if (i_w < 50) {
                        initData[i].push_back(i_w_vec[i]);
                        if (initData[i].size() > 10) {
                            initData[i].pop_front();
                        }
                    }
                    //目前还在通道，且体重数据未满，则加入到vec中
                    if (i_w >= 100 && weight_vec.at(i).size() <= 70) {
                        WEIGHT temp;
                        string curTime = getCurrentTime();
                        temp.weight = i_w;
                        temp.startTime = curTime;
                        temp.bar_col = bar.at(i);
                        weight_vec.at(i).push_back(temp);
                    } else if (i_w > 100 && weight_vec.at(i).size() > 70) {
                    } else if (i_w < 100 && weight_vec.at(i).size() > 5) {
                        //已经离开通道
                        thread thread_weight_handle(sqlStoreWeightThread, weight_vec.at(i));
                        thread_weight_handle.detach();
                        weight_vec.at(i).clear();
                        weightLog.log(bar[i] + ":    " + dequeToString(initData[i]));
                        initData[i].clear();
                        //计算时用的体重值
                    }
                }
            } catch (const std::exception &e) {
                error.log("种鹅经过设备时存储报错 " + string(e.what()));
            }
        }
    }catch (const std::exception &e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }catch (...) {
        // 捕获所有其他类型的异常
        std::cerr << "An unexpected error occurred" << std::endl;
    }
}

/**
 * @brief sql存储有效体重数据
 * @param weight_vec 有效体重数组
 */
void sqlStoreWeightThread(vector<WEIGHT> weight_vec) {
    unique_ptr<SQL_Conn> sql_instance = make_unique<SQL_Conn>();
    bool isConnect = sql_instance->connect();
    if (!isConnect) {
        return;
    }
    for (int i = 0; i < weight_vec.size(); i++) {
        string user_id = weight_vec[i].user_id;
        int weight = weight_vec[i].weight;
        string time = weight_vec[i].startTime;
        string column = weight_vec[i].bar_col;
        try {
            sql_instance->insert_originWeight(user_id, column, time, weight);
        } catch (const std::exception &e) {
            error.log("数据库SQL存储体重报错 " + string(e.what()));
        }
    }
}

int main() {
    thread rfid_thread(start_RFID_Thread);
    thread weight_thread(start_Weight_Thread);
    rfid_thread.join();
    weight_thread.join();
}
