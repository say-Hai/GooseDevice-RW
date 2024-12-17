
#include "Serial.h"

Serial::Serial(const char *com, size_t Baud):com(com),Baud(Baud) {}

Serial::~Serial() {
    if (hCom != INVALID_HANDLE_VALUE) {
        // 关闭串口句柄
        CloseHandle(hCom);
        hCom = INVALID_HANDLE_VALUE;
    }
}

void Serial::serialConnection() {
    hCom = CreateFile(com, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    isOpenCom(hCom);
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    GetCommState(hCom, &dcb);
    dcb.BaudRate = Baud;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    timeouts.ReadIntervalTimeout = MAXDWORD;  // 读取数据时不等待
    timeouts.ReadTotalTimeoutConstant = 0;    // 读取超时时间为0
    timeouts.ReadTotalTimeoutMultiplier = 0;  // 读取超时时间为0
    SetCommTimeouts(hCom, &timeouts);
    BOOL status = SetCommState(hCom, &dcb);
    isSetComState(status, hCom);
}

vector<uint8_t> Serial::getComData(vector<uint8_t> command) {
    size_t com_len = command.size();
    DWORD bytesWritten = 0;
    if (!WriteFile(hCom, reinterpret_cast<LPCVOID>(command.data()), com_len, &bytesWritten, nullptr)) {
        std::cerr << "指令发送失败." << std::endl;
        CloseHandle(hCom);
        return {};
    }
    // 等待一段时间以确保数据到达
    this_thread::sleep_for(chrono::milliseconds(100));
    std::vector<uint8_t> buffer(1024);
    DWORD bytesRead = 0;
    if (!ReadFile(hCom, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr)) {
        std::cerr << "数据读取失败." << std::endl;
        CloseHandle(hCom);
        return {};  // 返回空字符串表示失败
    }
    //接收到的数据
    buffer.resize(static_cast<uint8_t>(bytesRead));
    return buffer;
}

string Serial::read_weight_64() {
    //体重称无需发送指令，当串口连接时会不断地发送数据过来，我们只需要接收即可
    //一条完整的体重指令长度为64字节
    char buffer[65];
    DWORD bytesRead = 0;
    if (!ReadFile(hCom, buffer, 64, &bytesRead, nullptr)) {
        std::cerr << "数据读取失败." << std::endl;
        CloseHandle(hCom);
        return {};  // 返回空字符串表示失败
    }
    string response(buffer, bytesRead);
    return response;
}

vector<uint8_t> Serial::getComData(vector<uint8_t> command, const size_t expectedLength) {
    size_t com_len = command.size();
    DWORD bytesWritten = 0;

    // 异步写入数据
    if (!WriteFile(hCom, reinterpret_cast<LPCVOID>(command.data()), com_len, &bytesWritten, nullptr)) {
        std::cerr << "指令发送失败." << std::endl;
        CloseHandle(hCom);
        return {};
    }

    // 设置串口接收字符事件
    SetCommMask(hCom, EV_RXCHAR);
    DWORD eventMask;

    // 等待串口数据到达
    WaitCommEvent(hCom, &eventMask, nullptr);

    std::vector<uint8_t> buffer;
    DWORD bytesRead = 0;
    DWORD totalBytesRead = 0;
    COMSTAT comStat;
    DWORD errors;
    do {
        ClearCommError(hCom, &errors, &comStat);

        // 缓冲区中有字节等待接收
        if (comStat.cbInQue > 0) {
            std::vector<uint8_t> tempBuffer(comStat.cbInQue);
            if (!ReadFile(hCom, tempBuffer.data(), static_cast<DWORD>(tempBuffer.size()), &bytesRead, nullptr)) {
                std::cerr << "数据读取失败." << std::endl;
                CloseHandle(hCom);
                return {};
            }

            buffer.insert(buffer.end(), tempBuffer.begin(), tempBuffer.begin() + bytesRead);
            totalBytesRead += bytesRead;
        }
        // 等待更多数据
        this_thread::sleep_for(chrono::milliseconds(5));  // 防止CPU过高占用
    } while (totalBytesRead < expectedLength);

    return buffer;
}

vector<uint8_t> Serial::read_AnHuiRFID_data() {

    // Clear the serial port buffer
    PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    std::vector<uint8_t> buffer(1024);
    DWORD bytesRead = 0;

    if (!ReadFile(hCom, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr)) {
        std::cerr << "数据读取失败." << std::endl;
        CloseHandle(hCom);
        return {};  // 返回空字符串表示失败
    }

    // Resize the vector to the actual number of bytes read
    buffer.resize(static_cast<size_t>(bytesRead));
    return buffer;
}
