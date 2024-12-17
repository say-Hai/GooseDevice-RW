#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "RFID.h"
#include <windows.h>
class Logger
{
public:
	Logger(const std::string& filename);
	~Logger();
	void log(const std::string& message);
private:
	std::string m_filename;
	std::ofstream m_file;
};

