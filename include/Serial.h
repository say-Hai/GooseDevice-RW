
#ifndef TRANSDATATOSERVER_SERIAL_H
#define TRANSDATATOSERVER_SERIAL_H
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include<thread>
#include "UtilityClass.h"
using namespace std;
class Serial {
public:
    HANDLE get_h_com() const {
        return hCom;
    }


private:
    HANDLE hCom;
    DCB dcb;
    COMMTIMEOUTS timeouts = { 0 };
    const char * com;
    size_t Baud;
public:
    Serial(const char * com, size_t Baud);
    ~Serial();
    void serialConnection();
    vector<uint8_t> getComData(vector<uint8_t> command,  const size_t expectedLength);
    vector<uint8_t> Serial::getComData(vector<uint8_t> command);
    string read_weight_64();
    vector<uint8_t> read_AnHuiRFID_data();
};
#endif //TRANSDATATOSERVER_SERIAL_H
