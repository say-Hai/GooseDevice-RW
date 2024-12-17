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
#include "TransportToServer.h"

using namespace std;
mutex mtx;
vector<unsigned char> send_readCommand =	{ 0xBB, 0x17, 0x02, 0x00, 0x00, 0x19, 0x0D, 0x0A };		//多次查询
vector<unsigned char> send_readEND =		{ 0xBB, 0x18, 0x00, 0x18, 0x0D, 0x0A };					//关闭多次查询

void start_RFID_Thread(const char* com);

void start_RFID_Thread(const char* com) {
    mtx.lock();
    cout << "OpenRFID: ";
    unique_ptr<Serial> serRFID(new Serial(com, CBR_115200));
    vector<string> vec;
    TransportToServer *server = new TransportToServer(SERVERIP, RFIDPORT);
    mtx.unlock();
    unordered_map<pair<string, string>, vector<RFID>, pair_hash> epcBarKeyMap;
//1.开启多次查询
    serRFID->getComData(send_readCommand);
    while (true) {
        vector<uint8_t> receiveData =serRFID->read_AnHuiRFID_data();
//2.转成RFID数据
        vector<RFID> rfid_vec = anHui_dataToRFID(receiveData);
//3.对RFID数据按照{epc, bar}进行分组
        for (const auto& item: rfid_vec) {
            string temEpc = item.epc;
            string temBar = item.bar_col;
            int    temAnt = item.ant;
            pair<string, string> map_key = {temEpc, temBar};
            //如果上次读取和当前读取只有time不同，则只更新时间，不再增加vector的长度。
            if(epcBarKeyMap.count(map_key) != 0) {
                if(temAnt == epcBarKeyMap[map_key].back().ant){
                    epcBarKeyMap[map_key].back().startTime = item.startTime;
                }else {
                    epcBarKeyMap[map_key].push_back(item);
                }
            }else {
                epcBarKeyMap[map_key].push_back(item);
            }
        }
//4.遍历分组，找出已经离开设备的分组
//    4.1.CurrTime - 最后一次读取到该鹅的Time >= OUT_NESTLE,判定鹅已经离开通道
        for (auto item = epcBarKeyMap.begin(); item != epcBarKeyMap.end();) {
            string curTime = getCurrentTime();
            string epc_endTime = item->second.back().startTime;
            string timeGap = calcuTimeGap(epc_endTime, curTime);
            if (timeGap >= OUT_NESTLE) {
                if(com == "COM3") {
                    item->second.front().bar_col = bar.at(2);
                    item->second.back().bar_col = bar.at(2);
                }
                else if(com == "COM4") {
                    item->second.front().bar_col = bar.at(3);
                    item->second.back().bar_col = bar.at(3);
                }
                vector<RFID>temvec = {item->second.front(), item->second.back()};
                int ret = server->transportData(temvec);
                int err_num = 5;
                while(ret == -1 && err_num--) {
                    //再次重连
                    server = new TransportToServer(SERVERIP, RFIDPORT);
                    ret = server->transportData(temvec);
                    this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
                item = epcBarKeyMap.erase(item);
            } else item++;
        }
    }
    serRFID->getComData(send_readEND);
    return ;
}

int main() {
    const char* com = "COM3";
    cout<<com<<endl;
    thread rfid_thread(start_RFID_Thread, com);
    const char* com1 = "COM4";
    cout<<com1<<endl;
    thread rfid_thread1(start_RFID_Thread, com1);
    rfid_thread.join();
    rfid_thread1.join();
}