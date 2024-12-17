#include "UtilityClass.h"
#include "Logger.h"
#include "WeightReckon.h"
extern Logger weightLog;
extern Logger enterLog;
extern Logger exitLog;

/**
 * @brief 获取当前时间
 * @return 返回时间字符串
 */
std::string getCurrentTime() {
    // 获取当前时间点
    const auto now = std::chrono::system_clock::now();
    // 转换为time_t以便转换为tm结构
    auto time = std::chrono::system_clock::to_time_t(now);
    // 提取毫秒部分
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // 创建一个tm结构
    std::tm tm;
    // 使用localtime_s而不是localtime
    localtime_s(&tm, &time);

    // 使用stringstream进行格式化
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    // 添加毫秒部分
    ss << '.' << std::setw(3) << std::setfill('0') << milliseconds.count();

    return ss.str();
}

//A0 13 01 8A 每行指令必有的开头（HEAD + LEN + ADD + CMD）
//			 B3											  4C    8通道的ant计算
//				30 00 00 00 00 00 00 00 00 00 00 00 08 25		读取到的epc
//															66  check码
/**
 * @brief 解析RFID阅读器串口传输的指令
 * @param receive_data 串口数据指令
 * @return 解析后的RIFD结构体数组
 */
vector<RFID> dataToRFID(const vector<uint8_t> &receive_data) {
    vector<RFID> rfid_vec;
    auto iter = receive_data.begin();
    findHeadHex(receive_data, iter);
    while (iter != receive_data.end() && std::distance(iter, receive_data.end()) >= 18) {
        RFID rfid;
        //8通道的ant：iter+1的最低两位 + iter+16的最高位(最高位为1，则为5，6，7，8；最高位为0，则为1，2，3，4
        int cur_ant = (*(iter + 1) & 0x03) + 1 + (((*(iter + 16) & 0x80) > 0 ? 1 : 0) * 4);
        string cur_epc = EPCGXPref;
        cur_epc.append(hexToString(string(iter + 14, iter + 16)));//转换成字符串
        formatEPC(cur_epc);
        string cur_time = getCurrentTime();
        int bar_index = cur_ant / 2 + cur_ant % 2 - 1;
        string barCol = bar.at(bar_index);
        rfid.user_id = "1";
        rfid.ant = cur_ant;
        rfid.epc = cur_epc;
        rfid.startTime = cur_time;
        rfid.bar_col = barCol;
        rfid_vec.push_back(rfid);
        findHeadHex(receive_data, iter);
    }
    return rfid_vec;
}
vector<RFID> st_dataToRFID(const vector<uint8_t> &receive_data) {
    vector<RFID> rfid_vec;
    int data_len = receive_data.size();
    const vector<int> hexHead = { 0xBB, 0x96, 0x16, 0x30, 0x00 };
    auto iter = search(receive_data.begin(), receive_data.end(), hexHead.begin(), hexHead.end());

    while (iter != receive_data.end()) {
        int index = distance(receive_data.begin(), iter);
        if (data_len - index + 1 < 35) //一条指令是35，小于35则为异常数据（同时防止下面的代码越界）
            break;
        RFID rfid;
        int cur_ant = (receive_data[19 + index] ^ 0x03);
        string cur_epc = "EPCST001001";
        cur_epc.append(hexToString(string(iter + 15, iter + 17)));
        formatEPC(cur_epc);
        string cur_time = getCurrentTime();
        string barCol = "column_1";

        rfid.user_id = "3";
        rfid.ant = cur_ant;
        rfid.epc = cur_epc;
        rfid.startTime = cur_time;
        rfid.bar_col = barCol;
        rfid_vec.push_back(rfid);

        iter = search(iter + 5, receive_data.end(), hexHead.begin(), hexHead.end());
    }
    return rfid_vec;
}
//前缀：BB 97 1A
//PC：			14 00
//epc:				 00 00 05 27（iter + 4, iter + 4 + 3 + 1）
//冗余：				         E2 80 11 70 20 00 01 D0 34 7C 0A 4E
//RSSI：															 FD 27
//ant:																	  02 iter + 22
//冗余：																	20 25 90 C8
//Dir方向：																				00
//CRC:																						90 0D 0A
//校验码：																									  7E 7E 08 43 00 B2 F9
vector<RFID> anHui_dataToRFID(const vector<uint8_t> &receive_data) {
    vector<RFID> rfid_vec;
    const vector<int> hexHead = { 0x97, 0x1A };
    auto iter = search(receive_data.begin(), receive_data.end(), hexHead.begin(), hexHead.end()); //iter：指向每条指令的第一条头部指令
//循环所读到的指令，进行拆分
    while (iter != receive_data.end()) {
        int sub_len = distance(iter, receive_data.end());
        if (sub_len <= 22) {
            break;
        }
        string cur_epc = EPCAHPref;
        cur_epc.append(hexToString(string(iter + 6, iter + 8)));//转换成字符串
        formatEPC(cur_epc);
//        string epc_arr(iter + 4, iter + 8);
//        string curr_epc = hexToString(epc_arr);//当前EPC
//        formatEPC(curr_epc);
        int ant = *(iter + 22) % 2; //当前ANT

        RFID rfid_data;
        rfid_data.user_id = "2";
        rfid_data.ant = ant;
        rfid_data.epc = cur_epc;
        rfid_data.startTime = getCurrentTime();
        rfid_data.bar_col = bar.at(0);
        rfid_vec.push_back(rfid_data);
        iter = search(iter + 22, receive_data.end(), hexHead.begin(), hexHead.end()); //往下继续寻找
    }
    return rfid_vec;
}

/**
 * @brief 计算两者时间之差
 * @param start_time
 * @param end_time
 * @return 返回所差时间字符串
 */
std::string calcuTimeGap(const std::string &start_time, const std::string &end_time) {
    std::tm tm1 = {};
    std::tm tm2 = {};

    // 解析时间字符串1
    std::istringstream iss1(start_time);
    iss1 >> std::get_time(&tm1, "%Y-%m-%d %H:%M:%S");
    if (iss1.fail()) {
        // 解析失败，返回默认的时间差
        return "Invalid time format";
    }

    // 解析时间字符串2
    std::istringstream iss2(end_time);
    iss2 >> std::get_time(&tm2, "%Y-%m-%d %H:%M:%S");
    if (iss2.fail()) {
        // 解析失败，返回默认的时间差
        return "Invalid time format";
    }

    // 将解析后的时间转换为时间点
    std::chrono::system_clock::time_point timePoint1 = std::chrono::system_clock::from_time_t(std::mktime(&tm1));
    std::chrono::system_clock::time_point timePoint2 = std::chrono::system_clock::from_time_t(std::mktime(&tm2));

    // 计算时间差
    std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint2 - timePoint1);

    // 将时间差转换为 hh:mm:ss 格式的字符串
    std::chrono::seconds timeGap = std::chrono::duration_cast<std::chrono::seconds>(diff);
    int hours = timeGap.count() / 3600;
    int minutes = (timeGap.count() % 3600) / 60;
    int seconds = timeGap.count() % 60;

    // 构建时间差字符串
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setfill('0') << std::setw(2) << minutes << ":"
       << std::setfill('0') << std::setw(2) << seconds;

    return ss.str();
}

/**
 * @brief 16进制打印
 * @param input 16进制
 * @return 16进制字符串
 */
std::string hexToString(const std::string &input) {
    std::stringstream ss;
    for (char c: input) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(c);
    }
    return ss.str();
}

/**
 * @brief 查找每个指令的首部字段
 * @param receive_data 指令集
 * @param iter 当前遍历到的迭代器（引用）
 */
void findHeadHex(const std::vector<uint8_t> &receive_data,
                 _Vector_const_iterator<_Vector_val<_Simple_types<unsigned char>>>& iter) {
    int index = 0;
    while (iter != receive_data.end()) {
        if (*iter == HeadHex[index])
            index++;
        else {
            index = 0;
        }
        if (index == 4)
            return;
        iter++;
    }
    return;
}

void formatEPC(string &epc) {
    epc.erase(std::remove(epc.begin(), epc.end(), 'f'), epc.end());
}

/**
 * @brief 整理RFID数据，使其按EPC分组并按时间顺序排序
 * @param rfid_vec RFID结构体数组
 * @param epcBarKeyMap map表，对RFID进行分组排序
 */
void sortRFIDData(const vector<RFID> &rfid_vec,
                  unordered_map<pair<string, string>, vector<RFID>, pair_hash> &epcBarKeyMap)
{
    for (const auto &item: rfid_vec) {
        string temEpc = item.epc;
        string temBar = item.bar_col;
        int temAnt = item.ant;
        pair<string, string> map_key = {temEpc, temBar};
        //如果上次读取和当前读取只有time不同，则只更新时间，不再增加vector的长度。
        if (epcBarKeyMap.count(map_key) == 1 && epcBarKeyMap.at(map_key).size() >= 2) {
            if (temAnt == epcBarKeyMap[map_key].back().ant) {
                epcBarKeyMap[map_key].back().startTime = item.startTime;
            } else {
                epcBarKeyMap[map_key].push_back(item);
            }
        } else {
            epcBarKeyMap[map_key].push_back(item);
        }
    }
}

/**
 * @brief 检查串口是否打开
 * @param hcom 串口操作符
 */
void isOpenCom(HANDLE &hcom) {
    if (hcom == INVALID_HANDLE_VALUE) {
        cout << "串口打开失败，请重试" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "串口打开成功, ";
}

void isSetComState(bool status, HANDLE &hCom) {
    if (!status) {
        cout << " 串口状态参数设置失败 " << endl;
        CloseHandle(hCom);
        hCom = NULL;
        exit(EXIT_FAILURE);
    }
    cout << "串口状态参数设置成功" << endl;
}

//01 03 10
//			00 04 00 00 第一道称
//						00 00 00 03 第二道称
//									A9 EE 00 0F 第三道称
//												00 0F 00 00 第四道称
vector<int> dataToWEIGHT(vector<uint8_t> &receive_data) {
    vector<int> vec;
    int data_len = receive_data.size();
    if (data_len < 21)
        return {};
    if (checkHead(vector<uint8_t>(receive_data.begin(), receive_data.begin() + 3))) {
        for (int i = 0; i < 4; i++) {
            int i_w = receive_data.at(i * 4 + 6) * pow(256, 0);
            if(i_w > 20000)
                continue;
            i_w += receive_data.at(i * 4 + 5) * pow(256, 1);
            if(i_w > 20000)
                continue;
            i_w += receive_data.at(i * 4 + 4) * pow(256, 2);
            if(i_w > 20000)
                continue;
            i_w += receive_data.at(i * 4 + 3) * pow(256, 3);
            if(i_w > 20000)
                continue;
            vec.push_back(i_w);
        }
    }
    return vec;
}

bool checkHead(vector<uint8_t> &&receive_Head) {
    return receive_Head == headHex;
}


void isEnterOrExit(vector<RFID> rfid_data_arr) {
    if(rfid_data_arr.size() < 2)
        return ;
    unique_ptr<SQL_Conn> sql_instance = make_unique<SQL_Conn>();
    bool isConncet = sql_instance->connect();
    if(!isConncet) {
        return;
    }
    for (auto item: rfid_data_arr) {
        sql_instance->insert_originRFID_Data(item.user_id, item.bar_col, to_string(item.ant), item.epc, item.startTime);
    }
    string USER_ID = rfid_data_arr.begin()->user_id;
    string DEVICE_COL = rfid_data_arr.begin()->bar_col;
    string target_epc = rfid_data_arr.begin()->epc;

    string begin_time = rfid_data_arr.begin()->startTime;
    int begin_ant = rfid_data_arr.begin()->ant % 2;

    string end_time = rfid_data_arr.back().startTime;
    int end_ant = rfid_data_arr.back().ant % 2;
    int OUT_INDEX = 1;
    int IN_INDEX = 0;
    if (begin_ant == OUT_INDEX && end_ant == IN_INDEX) {
        vector<double> weights = sql_instance->select_WeightInTime(USER_ID, DEVICE_COL, begin_time, end_time);
        if(weights.size() < 5) {
            weightLog.log(DEVICE_COL + "   采集的数据不够 ");
            return;
        }
        vector<double> temW = weights;
        int predict_w  = getWeightItem(weights, target_epc);
        if(predict_w == 0) {
            return;
        }
        enterLog.log("originData:   " + target_epc +", " + DEVICE_COL + vectorToString(temW));
        sql_instance->insert_weight_change_record(USER_ID, DEVICE_COL, target_epc, end_time, predict_w);
        sql_instance->insert_EnterOrExit_Data(USER_ID, DEVICE_COL, target_epc, begin_time, end_time, "enter", predict_w);
    }
    //出去 1->0
    else if (begin_ant == IN_INDEX && end_ant == OUT_INDEX) {
        vector<double> weights = sql_instance->select_WeightInTime(USER_ID, DEVICE_COL, begin_time, end_time);
        if(weights.size() < 5) {
            weightLog.log(DEVICE_COL + "   采集的数据不够 ");
            return;
        }
        vector<double> temW = weights;
        int predict_w  = getWeightItem(weights, target_epc);
        if(predict_w == 0) {
            return;
        }
        exitLog.log("originData:   " + target_epc +", " + DEVICE_COL + vectorToString(temW));
        sql_instance->insert_weight_change_record(USER_ID, DEVICE_COL, target_epc, end_time, predict_w);
        sql_instance->insert_EnterOrExit_Data(USER_ID, DEVICE_COL, target_epc, begin_time, end_time, "exit", predict_w);
    }
}

void coutInitWeight(const deque<int> initData) {
    for(const int &i: initData) {
        cout << i <<"   ";
    }
}

std::string dequeToString(const std::deque<int> &dq) {
    string str = "[ ";
    for(auto item: dq) {
        str += to_string(item);
        str += " ";
    }
    str += "]";
    return str;
}
