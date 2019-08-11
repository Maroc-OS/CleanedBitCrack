#ifndef _COMMON_UTIL_H
#define _COMMON_UTIL_H

#include <string>
#include <portable_endian.h>

namespace CommonUtils {
class Timer {
private:
	uint64_t _startTime;

public:
	Timer();

	void start();

	uint64_t getTime();
};

uint64_t getSystemTime();

void commonUtilsSleep(int seconds);

std::string formatThousands(uint64_t x);

std::string formatThousands(std::string s);

std::string formatSeconds(unsigned int seconds);

uint32_t parseUInt32(const std::string &s);

uint64_t parseUInt64(std::string s);

bool isHex(const std::string &s);

bool appendToFile(const std::string &fileName, const std::string &s);

bool readLinesFromStream(std::istream &in, std::vector<std::string> &lines);

bool readLinesFromStream(const std::string &fileName,
		std::vector<std::string> &lines);

std::string format(const char *formatStr, double value);

std::string format(uint32_t value);

std::string format(uint64_t value);

std::string format(int value);

void removeNewline(std::string &s);

std::string toLower(const std::string &s);

std::string trim(const std::string &s, char c = ' ');

// https://stackoverflow.com/questions/10605342/converting-a-long-hexadecimal-string-to-a-decimal-string
class HexToDecString {
public:
	static std::string convert(std::string in);

private:
#define H2D_MAXLEN 1000
	struct number {
		unsigned char digits[H2D_MAXLEN];
		unsigned int num_digits;
	};

	static void copy_number(struct number *dst, struct number *src);

	static int add(struct number *, struct number *, struct number *);

	static int mult(struct number *, struct number *, struct number *);

	static int power(struct number *, unsigned int, struct number *);

	static void dec(struct number *);
};
} // namespace CommonUtils

#endif
