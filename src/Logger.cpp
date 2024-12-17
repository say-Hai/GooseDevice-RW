#include "Logger.h"
#include "UtilityClass.h"
using namespace std;

std::wstring getCurrentDate() {
	// 获取当前时间
	auto now = std::chrono::system_clock::now();
	std::time_t time = std::chrono::system_clock::to_time_t(now);

	// 格式化时间为 "YYYY-MM-DD"
	std::wstringstream ss;
	ss << std::put_time(std::localtime(&time), L"%Y-%m-%d");

	return ss.str();
}

/**
 * @brief 将日志信息存储到对应的文件中
 * @param filename 日志信息
 */
Logger::Logger(const std::string& filename) :m_filename(filename)
{
	// 1. 获取当前日期，拼接文件夹路径
	std::wstring folderPath = L"Logger\\" + getCurrentDate();

	// 2. 创建文件夹
	CreateDirectoryW(folderPath.c_str(), NULL);

	// 3. 将 std::string 文件名转换为 std::wstring，并拼接完整路径
	std::wstring wfilename(filename.begin(), filename.end()); // 转换 std::string 为 std::wstring
	std::wstring fullPath = folderPath + L"\\" + wfilename; // 拼接文件夹路径和文件名

	// 4. 打开文件
	m_file.open(std::string(fullPath.begin(), fullPath.end()), std::ios::app);  // 使用追加模式打开文件
}

Logger::~Logger()
{
	if (m_file.is_open()) {
		m_file.close();
	}
}

void Logger::log(const std::string& message)
{
	if (m_file.is_open()) {
		m_file << getCurrentTime() << "        " << message << std::endl;
	}
}
Logger weightLog("weightLog.txt");
Logger rfidLog("rfidLog.txt");
Logger enterLog("enterLog.txt");
Logger exitLog("exitLog.txt");
Logger error("error.txt");


