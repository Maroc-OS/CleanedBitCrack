#include "CommonUtils.h"
#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <stdexcept>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#ifdef __cplusplus
}
#endif

using namespace std;

namespace CommonUtils {
uint64_t getSystemTime() {
#ifdef _WIN32
    return GetTickCount64();
#else
	struct timeval t;
	gettimeofday(&t, nullptr);
    return static_cast<uint64_t>(t.tv_sec * 1000) + static_cast<uint64_t>(t.tv_usec / 1000);
#endif
}

Timer::Timer() {
	_startTime = 0;
}

void Timer::start() {
	_startTime = getSystemTime();
}

uint64_t Timer::getTime() {
	return getSystemTime() - _startTime;
}

void commonUtilsSleep(int seconds) {
#ifdef _WIN32
	Sleep(seconds * 1000);
#else
	sleep(static_cast<unsigned int>(seconds));
#endif
}

std::string formatThousands(uint64_t x) {
	char buf[32] = "";

	snprintf(buf, 32, "%" PRIu64 "", x);

	std::string s(buf);

	return formatThousands(s);
}

std::string formatThousands(std::string s) {
	int len = static_cast<int>(s.length());

	int numCommas = (len - 1) / 3;

	if (numCommas == 0) {
		return s;
	}

	std::string result = "";

	int count = ((len % 3) == 0) ? 0 : (3 - (len % 3));

	for (int i = 0; i < len; i++) {
		result += s[static_cast<size_t>(i)];

		if (count++ == 2 && i < len - 1) {
			result += ",";
			count = 0;
		}
	}

	return result;
}

uint32_t parseUInt32(const std::string &s) {
	  uint64_t num = parseUInt64(s);
	  if ((num >> 32LL) == 0) {
	    return static_cast<uint32_t>(num);
	  } else {
	    throw std::out_of_range(s);
	  }
}

uint64_t parseUInt64(std::string s) {
	uint64_t val = 0;
	bool StringIsHex = false;

	if (s[0] == '0' && s[1] == 'x') {
		StringIsHex = true;
		s = s.substr(2);
	}

	if (s[s.length() - 1] == 'h') {
		StringIsHex = true;
		s = s.substr(0, s.length() - 1);
	}

	if (StringIsHex) {
		if (sscanf(s.c_str(), "%" PRIx64 "", &val) != 1) {
			throw std::runtime_error("Expected an integer");
		}
	} else {
		if (sscanf(s.c_str(), "%" PRIu64 "", &val) != 1) {
			throw std::runtime_error("Expected an integer");
		}
	}

	return val;
}

bool isHex(const std::string &s) {
	int len = 0;

	for (int i = 0; i <= len; i++) {
		char c = s[static_cast<size_t>(i)];

		if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
				|| (c >= 'A' && c <= 'F'))) {
			return false;
		}
	}

	return true;
}

std::string formatSeconds(unsigned int seconds) {
	char s[128] = { 0 };

	unsigned int days = seconds / 86400;
	unsigned int hours = (seconds % 86400) / 3600;
	unsigned int minutes = (seconds % 3600) / 60;
	unsigned int sec = seconds % 60;

	if (days > 0) {
		snprintf(s, 128, "%" PRIu32 ":%02" PRIu32 ":%02" PRIu32 ":%02" PRIu32 "", days, hours, minutes, sec);
	} else {
		snprintf(s, 128, "%02" PRIu32 ":%02" PRIu32 ":%02" PRIu32 "", hours, minutes, sec);
	}

	return std::string(s);
}

std::ifstream::pos_type getFileSize(const std::string& fileName)
{
    std::ifstream in(fileName, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

/* long getFileSize(const std::string &fileName) {
	FILE *fp = fopen(fileName.c_str(), "rb");
	if (fp == NULL) {
		return -1;
	}

	fseek(fp, 0, SEEK_END);

	long pos = ftell(fp);

	fclose(fp);

	return pos;
}*/

bool readLinesFromStream(const std::string &fileName,
		std::vector<std::string> &lines) {
	std::ifstream inFile(fileName.c_str());

	if (!inFile.is_open()) {
		return false;
	}

	return readLinesFromStream(inFile, lines);
}

bool readLinesFromStream(std::istream &in, std::vector<std::string> &lines) {
	const int MAX_LENGTH = 256;
	char* line = new char[MAX_LENGTH];
	while (in.getline(line, MAX_LENGTH) && strlen(line) > 0) {
		lines.push_back(line);
	}

	return true;
}

bool appendToFile(const std::string &fileName, const std::string &s) {
	std::ofstream outFile;
	bool newline = false;

	if (getFileSize(fileName) > 0) {
		newline = true;
	}

	outFile.open(fileName.c_str(), std::ios::app);

	if (!outFile.is_open()) {
		return false;
	}

	// Add newline following previous line
	if (newline) {
		outFile << std::endl;
	}

	outFile << s;

	return true;
}

std::string format(const char *formatStr, double value) {
	char buf[100] = { 0 };

	snprintf(buf, 100, formatStr, value);

	return std::string(buf);
}

std::string format(uint32_t value) {
	char buf[100] = { 0 };

	snprintf(buf, 100, "%u", value);

	return std::string(buf);
}

std::string format(uint64_t value) {
	char buf[100] = { 0 };

	snprintf(buf, 100, "%" PRIu64 "", value);

	return std::string(buf);
}

std::string format(int value) {
	char buf[100] = { 0 };

	snprintf(buf, 100, "%d", value);

	return std::string(buf);
}

void removeNewline(std::string &s) {
	size_t len = s.length();

	size_t toRemove = 0;

	if (len >= 2) {
		if (s[len - 2] == '\r' || s[len - 2] == '\n') {
			toRemove++;
		}
	}
	if (len >= 1) {
		if (s[len - 1] == '\r' || s[len - 1] == '\n') {
			toRemove++;
		}
	}

	if (toRemove) {
		s.erase(len - toRemove);
	}
}

std::string toLower(const std::string &s) {
	std::string lowerCase = s;
	std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(),
			::tolower);

	return lowerCase;
}

std::string trim(const std::string &s, char c) {
	size_t left = s.find_first_not_of(c);
	size_t right = s.find_last_not_of(c);

	return s.substr(left, right - left + 1);
}

int HexToDecString::add(struct number *a, struct number *b, struct number *c) {
	int j;
	int carry;
	unsigned int i;

	struct number *t;

	if (a->num_digits < b->num_digits) {
		t = a;
		a = b;
		b = t;
	}

	for (i = 0, carry = 0; i < a->num_digits; i++) {
		if (i >= b->num_digits) {
			j = a->digits[i] + carry;
		}
		else {
			j = a->digits[i] + b->digits[i] + carry;
		}

		if (j > 9) {
			j -= 10;
			carry = 1;
		} else {
			carry = 0;
		}

		c->digits[i] = static_cast<unsigned char>(j);
	}

	/* Did we overflow? */
	if (carry > 0 && i == H2D_MAXLEN) {
		return -1;
	}

	/* Carry over from last addition? */
	if (carry > 0) {
		c->digits[i] = static_cast<unsigned char>(carry);
		c->num_digits = a->num_digits + 1;
	} else {
		c->num_digits = a->num_digits;
	}

	return 0;
}

void HexToDecString::copy_number(struct number *dst, struct number *src) {
	for (unsigned int i = 0; i < src->num_digits; i++) {
		dst->digits[i] = src->digits[i];
	}

	dst->num_digits = src->num_digits;
}

int HexToDecString::power(struct number *a, unsigned int n, struct number *b) {
	struct number atmp;

	/* Are we exponentiating by 0? */
	if (n == 0) {
		b->num_digits = 1;
		b->digits[0] = 1;
		return 0;
	}

	copy_number(&atmp, a);

	while (--n > 0) {
		mult(&atmp, a, &atmp);
	}

	copy_number(b, &atmp);
	return 0;
}

void HexToDecString::dec(struct number *a) {
	unsigned int i ,j;

	for (i = 0; i < a->num_digits; i++) {
		if (a->digits[i] > 0) {
			a->digits[i]--;
			break;
		}

		a->digits[i] = 9;
	}

	/* Did number of digits get lower */
	if (i == a->num_digits - 1 && a->digits[i] == 0) {
		for (j = a->num_digits, i = j - 1; j > 0; j--, i = j - 1){
			if (a->digits[i] != 0) {
				a->num_digits = i + 1;
				break;
			}
		}
	}
}

int HexToDecString::mult(struct number *a, struct number *b, struct number *c) {
	struct number btmp;
	struct number ctmp;
	struct number *t;

	/* Are we multiplying by 0? */
	if (a->num_digits == 0 || b->num_digits == 0) {
		c->num_digits = 0;
		return 0;
	}

	if (a->num_digits < b->num_digits) {
		t = a;
		a = b;
		b = t;
	}

	copy_number(&btmp, b);
	copy_number(&ctmp, a);

	while (1) {
		/* Are we multiplying by 1? */
		if (btmp.num_digits == 1 && btmp.digits[0] == 1) {
			break;
		}

		add(&ctmp, a, &ctmp);
		dec(&btmp);
	}

	copy_number(c, &ctmp);

	return 0;
}

std::string HexToDecString::convert(std::string in) {
	int n;
	struct number decrep;
	struct number twopow;
	struct number digit;
	std::string out = "";

	decrep.num_digits = 0;
	n = static_cast<int>(in.length());

	unsigned int currentChar = 0;
	while (--n > -1) {
		/* weight of digit */
		twopow.num_digits = 2;
		twopow.digits[0] = 6;
		twopow.digits[1] = 1;

		power(&twopow, static_cast<unsigned int>(n), &twopow);

		char s = in.at(currentChar++);

		/* Extract digit */
		if (s == '0') {
			digit.digits[0] = s - '0';
			digit.num_digits = 0;
		} else if (s <= '9' && s > '0') {
			digit.digits[0] = s - '0';
			digit.num_digits = 1;
		} else if (s <= 'F' && s >= 'A') {
			digit.digits[0] = s - 'A';
			digit.digits[1] = 1;
			digit.num_digits = 2;
		}

		mult(&digit, &twopow, &digit);
		add(&decrep, &digit, &decrep);
	}

	/* Convert decimal number to a string */
	if (decrep.num_digits == 0) {
		return out;
	}

	for (n = static_cast<int>(decrep.num_digits) - 1; n >= 0; n--) {
		out += static_cast<char>('0') + decrep.digits[n];
	}

	return out;
}
} // namespace commonUtilsSleep
