#ifndef _LOGGER_H
#define _LOGGER_H

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace LogLevel {
enum Level {
	Info = 1, Error = 2, Debug = 4, Warning = 8, Notify = 16
};

bool isValid(int level);

std::string toString(int level);
}

class Logger {
private:
	static std::string _logFile;

	static std::string formatLog(LogLevel::Level logLevel, const std::string &msg);

	static std::string getDateTimeString();

public:
	Logger() {
	}

	static void log(LogLevel::Level Level, const std::string &msg);

	static void setLogFile(const std::string &path);
};

#endif
