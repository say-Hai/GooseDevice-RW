#ifndef TRANSDATATOSERVER_UTILITYCLASS_H
#define TRANSDATATOSERVER_UTILITYCLASS_H


#include <iostream>
#include <windows.h>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include "RFID.h"
#include "WEIGHT.h"
#include "Defines.h"
#include <thread>
#include "SQL_Conn.h"
#include <deque>


//-------------------------------RFID-------------------------------
//将读写器原始数据解析成RFID数据
const std::vector<std::string> bar = {"column_1", "column_2", "column_3", "column_4"};
vector<RFID> dataToRFID(const vector<uint8_t> &receive_data);
vector<RFID> anHui_dataToRFID(const vector<uint8_t> &receive_data);
vector<RFID> st_dataToRFID(const vector<uint8_t> &receive_data);
//寻找读写器头部指令
const vector<uint8_t> HeadHex = {0xA0, 0x13, 0x01, 0x8A};
inline void findHeadHex(const std::vector<uint8_t> &receive_data,
                      _Vector_const_iterator<_Vector_val<_Simple_types<unsigned char>>>& iter);
//格式化EPC格式
inline void formatEPC(string& epc);
//对RFID分组
void sortRFIDData(const vector<RFID>& rfid_vec, unordered_map<pair<string, string>, vector<RFID>, pair_hash>& epcBarKeyMap);
//发送到数据库
void isEnterOrExit(vector<RFID> rfid_data_arr);
//-------------------------------通用-------------------------------
//计算两个时间之差
std::string calcuTimeGap(const std::string& start_time, const std::string& end_time);
//获取当前时间字符串
string getCurrentTime();
//将16进制转成字符串（0x10 -> "10"）
inline string hexToString(const std::string &input);
//-------------------------------串口-------------------------------
//检查串口是否打开
void isOpenCom(HANDLE& hcom);
//检查串口参数是否设置成功
void isSetComState(bool status, HANDLE& hCom);
//-------------------------------体重-------------------------------
//将原始数据解析成体重数据
vector<int> dataToWEIGHT(vector<uint8_t>& receive_data);
vector<double> getWeight(string recv_weight);

inline vector<double> getWeight(string receive_data) {
    std::vector<std::string> lines;
    vector<double> weights;
    size_t pos = 0;
    while ((pos = receive_data.find("\n")) != std::string::npos) {
        std::string line = receive_data.substr(0, pos);
        lines.push_back(line);
        receive_data.erase(0, pos + 1);
    }
    lines.push_back(receive_data);
    for (string& line : lines) {
        size_t pos = 0;
        std::vector<std::string> split_data;
        while ((pos = line.find(",")) != std::string::npos) {
            std::string data = line.substr(0, pos);
            split_data.push_back(data);
            line.erase(0, pos + 1);
        }
        split_data.push_back(line);
        string third_data = split_data[2];
        size_t start_pos = third_data.find("+");
        if (start_pos != string::npos) {
            std::string trimmed_data = third_data.substr(start_pos + 1);
            double parsed_weight = std::stof(trimmed_data);
            parsed_weight = round(parsed_weight * 100.0) / 100.0; //round to two decimal places
            weights.push_back(parsed_weight);
        }
    }
    return weights;
}

//检查头部
const vector<uint8_t> headHex = { 0x01, 0x03, 0x10 };
inline bool checkHead(vector<uint8_t>&& receive_Head);
void coutInitWeight(const deque<int>initData);
std::string dequeToString(const std::deque<int>& dq);

string vectorToString(const vector<double>& vec);

inline string vectorToString(const vector<double> &vec) {
    std::ostringstream oss;
    oss <<"[ ";
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << static_cast<int>(vec[i]); // 将 double 转换为 int
        if (i != vec.size() - 1) {
            oss << ", "; // 元素之间用逗号分隔
        }
    }
    oss <<" ]";
    return oss.str();
}
#endif //TRANSDATATOSERVER_UTILITYCLASS_H

