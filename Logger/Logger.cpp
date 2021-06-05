#include "Logger.h"
#include <time.h>

bool LogLevel::isValid(int level) {
	switch (level) {
	case Info:
	case Error:
	case Debug:
		return true;
	default:
		return false;
	}
}

std::string LogLevel::toString(int level) {
	switch (level) {
		case Info:
			return "Info";
		case Error:
			return "Error";
		case Debug:
			return "Debug";
		case Warning:
			return "Warning";
		default:
			return "";
	}
}

std::string Logger::getDateTimeString() {
	std::stringstream ss;
	std::time_t t = std::time(nullptr);
#ifdef _WIN32
	ss << std::put_time(gmtime(&t), "%Y-%m-%d.%X");
#else
	struct std::tm lt = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	ss << std::put_time(gmtime_r(&t, &lt), "%Y-%m-%d.%X");
#endif

	return std::string(ss.str());
}

std::string Logger::formatLog(int logLevel, const std::string &msg) {
	std::string dateTime = getDateTimeString();

	std::string prefix = "[" + dateTime + "] [" + LogLevel::toString(logLevel)
			+ "] ";

	size_t prefixLen = prefix.length();

	std::string padding(prefixLen, ' ');

	if (msg.find('\n', 0) != std::string::npos) {
		size_t pos = 0;
		size_t prev = 0;

		while ((pos = msg.find('\n', prev)) != std::string::npos) {
			prefix += msg.substr(prev, pos - prev) + "\n" + padding;
			prev = pos + 1;
		}

		prefix += msg.substr(prev);
	} else {
		prefix += msg;
	}

	return prefix;
}

void Logger::log(int logLevel, const std::string &msg) {
	std::string str = formatLog(logLevel, msg);

	fprintf(stderr, "%s\n", str.c_str());
}

void Logger::setLogFile(const std::string &path) {
	fprintf(stdin, "%s\n", path.c_str());
}
