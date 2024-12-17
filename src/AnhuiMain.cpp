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

using namespace std;
mutex mtx;
extern Logger rfidLog;
vector<unsigned char> send_readCommand =	{ 0xBB, 0x17, 0x02, 0x00, 0x00, 0x19, 0x0D, 0x0A };		//多次查询
vector<unsigned char> send_readEND =		{ 0xBB, 0x18, 0x00, 0x18, 0x0D, 0x0A };					//关闭多次查询

void start_RFID_Thread();

void start_RFID_Thread() {
    cout << "OpenRFID: ";
    unique_ptr<Serial> serRFID = make_unique<Serial>(RFIDCOM, CBR_115200);
    serRFID->serialConnection(); //连接串口
    unordered_map<pair<string, string>, vector<RFID>, pair_hash> epcBarKeyMap;
//1.开启多次查询
    serRFID->getComData(send_readCommand);
    while (true) {
        vector<uint8_t> receiveData =serRFID->read_AnHuiRFID_data();
//2.转成RFID数据
        vector<RFID> rfid_vec = anHui_dataToRFID(receiveData);
//3.对RFID数据按照{epc, bar}进行分组
        sortRFIDData(rfid_vec, epcBarKeyMap);
//4.遍历分组，找出已经离开设备的分组
//    4.1.CurrTime - 最后一次读取到该鹅的Time >= OUT_NESTLE,判定鹅已经离开通道
        for (auto item = epcBarKeyMap.begin(); item != epcBarKeyMap.end();) {
            string curTime = getCurrentTime();
            string epc_endTime = item->second.back().startTime;
            string timeGap = calcuTimeGap(epc_endTime, curTime);
            if (timeGap >= OUT_NESTLE) {
                vector<RFID>temvec = {item->second.front(), item->second.back()};
                rfidLog.log(temvec.front().epc + ": " +
                            to_string(temvec.front().ant) + " --》 " + to_string(temvec.back().ant));
                isEnterOrExit(temvec);
                item = epcBarKeyMap.erase(item);
            } else item++;
        }
    }
    serRFID->getComData(send_readEND);
    return ;
}

int main() {
    cout<<"AnHui    "<<RFIDCOM<<endl;
    thread rfid_thread(start_RFID_Thread);
    rfid_thread.join();
}