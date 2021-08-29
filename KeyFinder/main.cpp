#include <cinttypes>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "AddressUtil.h"
#include "CmdParse.h"
#include "CommonUtils.h"
#include "KeyFinder.h"
#include "KeySearchDevice.h"
#include "KeySearchTypes.h"
#include "Logger.h"
#include "secp256k1.h"
#include "ConfigFile.h"
#include "DeviceManager.h"

#ifdef BUILD_CUDA
#include "CudaKeySearchDevice.h"
#endif

#ifdef BUILD_OPENCL
#include "CLKeySearchDevice.h"
#endif

typedef struct RunConfig {
	// startKey is the first key. We store it so that if the --continue
	// option is used, the correct progress is displayed. startKey and
	// nextKey are only equal at the very beginning. nextKey gets saved
	// in the checkpoint file.
	secp256k1::uint256 startKey = 1;
	secp256k1::uint256 nextKey = 1;

	// The last key to be checked
	secp256k1::uint256 endKey = secp256k1::N - 1;

	secp256k1::uint256 stride = 1;

	uint64_t totalkeys = 0;
	uint64_t statusInterval = 1800;
	uint64_t checkpointInterval = 60000;

	unsigned int threads = 64;
	unsigned int blocks = 32;
	unsigned int pointsPerThread = 1024;

	unsigned int elapsed = 0;

	std::vector<std::string> targets;

	std::string targetsFile = "";
	std::string checkpointFile = "";
	std::string resultsFile = "";

	int compression = PointCompressionType::COMPRESSED;
	int device = 0;

	bool randomMode = false;
	bool follow = false;
} RunConfig;

static RunConfig _config;

std::vector<DeviceManager::DeviceInfo> _devices;

void writeCheckpoint(secp256k1::uint256 nextKey);

static uint64_t _lastUpdate = 0;
static uint64_t _startTime = 0;

/**
 * Callback to display the private key
 */
void resultCallback(KeySearchResult info);
void resultCallback(KeySearchResult info)
    {
	if (_config.resultsFile.length() != 0) {
		Logger::log(LogLevel::Info,
				"Found key for address '" + info.address + "'. Written to '"
						+ _config.resultsFile + "'");

		std::string s = info.address + " " + info.privateKey.toString(16) + " "
				+ info.publicKey.toString(info.compressed);
		CommonUtils::appendToFile(_config.resultsFile, s);

		return;
	}

	std::string logStr = "\nAddress:     " + info.address + "\n";
	logStr += "Private key: " + info.privateKey.toString(16) + "\n";
	logStr += "Compressed:  ";

	if (info.compressed) {
		logStr += "yes\n";
	} else {
		logStr += "no\n";
	}

	logStr += "Public key:  \n";

	if (info.compressed) {
		logStr += info.publicKey.toString(true) + "\n";
	} else {
		logStr += info.publicKey.x.toString(16) + "\n";
		logStr += info.publicKey.y.toString(16) + "\n";
	}

	Logger::log(LogLevel::Info, logStr);
	Logger::log(LogLevel::Notify, logStr);
}

typedef struct {
	std::string singular;
	std::string plural;
	secp256k1::uint256 factor;
	int forcedValue;
} time_conversion_t;

typedef std::vector<time_conversion_t> t_time_conversion_t;

secp256k1::uint256 FACTOR_MINUTE(60);
secp256k1::uint256 FACTOR_HOUR(FACTOR_MINUTE.mul(60));
secp256k1::uint256 FACTOR_DAY(FACTOR_HOUR.mul(24));
secp256k1::uint256 FACTOR_WEEK(FACTOR_DAY.mul(7));
secp256k1::uint256 FACTOR_MONTH(FACTOR_DAY.mul(31));
secp256k1::uint256 FACTOR_YEAR(FACTOR_DAY.mul(365));
secp256k1::uint256 FACTOR_DECADE(FACTOR_YEAR.mul(10));
secp256k1::uint256 FACTOR_CENTURY(FACTOR_DECADE.mul(10));
secp256k1::uint256 FACTOR_MILLENIUM(FACTOR_CENTURY.mul(10));
secp256k1::uint256 FACTOR_MILLENIUM_AND_MORE(FACTOR_MILLENIUM.mul(100000000));
secp256k1::uint256 FACTOR_END(
		"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

t_time_conversion_t timeConversions = {
	{ "second", "seconds", 1, -1 },
	{ "minute", "minutes", FACTOR_MINUTE, -1 },
	{ "hour", "hours", FACTOR_HOUR, -1 },
	{ "day", "days", FACTOR_DAY, -1 },
	{ "week", "weeks", FACTOR_WEEK, -1 },
	{ "month", "months", FACTOR_MONTH, -1 },
	{ "year", "years", FACTOR_YEAR, -1 },
	{ "decade", "decades", FACTOR_DECADE, -1 },
	{ "century", "centuries", FACTOR_CENTURY, -1 },
	{ "millenium", "milleniums", FACTOR_MILLENIUM, -1 },
	{ "millenium", "milleniums and more", FACTOR_MILLENIUM_AND_MORE, 100000000 },
	{ "END", "END", FACTOR_END, -1 },
};

void getTimeRemaining(secp256k1::uint256 &timeInSeconds, double &outDecimalTime,
		std::string &outUnit);
void getTimeRemaining(secp256k1::uint256 &timeInSeconds, double &outDecimalTime,
		std::string &outUnit) {
	for (size_t i = 0; i < timeConversions.size() - 1; i++) {
		time_conversion_t *timeConversion = &timeConversions[i];
		time_conversion_t *timeConversionNext = &timeConversions[i + 1];

		if (timeInSeconds.cmp(timeConversionNext->factor) < 0) {
			outUnit = timeConversion->singular;
			outDecimalTime = (double) timeInSeconds.toUint64()
					/ (double) timeConversion->factor.toUint64();

			if (timeInSeconds.cmp(1) != 0) {
				outUnit = timeConversion->plural;
			}

			if (timeConversion->forcedValue != -1)
				outDecimalTime = timeConversion->forcedValue;

			break;
		}
	}
}

/**
 Callback to display progress
 */
void statusCallback(const KeySearchStatus info);
void statusCallback(const KeySearchStatus info) {
	std::string speedStr;

	if (info.speed < 0.01) {
		speedStr = "< 0.01 MKey/s";
	} else {
		speedStr = CommonUtils::format("%.2f", info.speed) + " MKey/s";
	}

	secp256k1::uint256 totalRangeKeys = _config.endKey.sub(_config.nextKey);
	secp256k1::uint256 totalProcessed(info.total);
	secp256k1::uint256 remainingKeys = totalRangeKeys.sub(totalProcessed);

	std::string totalStr;

	if (_config.randomMode) {
		totalStr = "("
				+ CommonUtils::formatThousands(_config.totalkeys + info.total)
				+ " total)";
	} else {
		totalStr = "("
				+ CommonUtils::formatThousands(
						remainingKeys.toString(10, false)) + " remaining)";
	}

	std::string timeStr = "["
			+ CommonUtils::formatSeconds(
					_config.elapsed + static_cast<unsigned int>(info.totalTime) / 1000)
			+ "]";

	std::string timeRemainingStr;

	if (_config.randomMode) {
		timeRemainingStr = "infinity";

	} else {
		secp256k1::uint256 timeRemainingSeconds = remainingKeys.div(
				static_cast<unsigned int>(info.speed) * 1000000);

		double timeRemaining = 0;
		std::string timeUnit("second");

		getTimeRemaining(timeRemainingSeconds, timeRemaining, timeUnit);

		timeRemainingStr = "[ETA " + CommonUtils::format("%.2f", timeRemaining)
				+ " " + timeUnit.c_str() + "]";
	}

	std::string usedMemStr = CommonUtils::format(
			(info.deviceMemory - info.freeMemory) / (1024 * 1024));

	std::string totalMemStr = CommonUtils::format(
			info.deviceMemory / (1024 * 1024));

	std::string targetStr = CommonUtils::format(info.targets) + " target"
			+ (info.targets > 1 ? "s" : "");

	// Fit device name in 16 characters, pad with spaces if less
	/* std::string devName = info.deviceName.substr(0, 16);
	devName += std::string(16 - devName.length(), ' '); */

	const char *formatStr = nullptr;

	if (_config.follow) {
		formatStr = "%s %s/%sMB | %s %s %s %s\n";
	} else {
		formatStr = "\r%s %s/%sMB | %s %s %s %s";
	}

	printf(formatStr, timeStr.c_str(), usedMemStr.c_str(), totalMemStr.c_str(),
			targetStr.c_str(), speedStr.c_str(), totalStr.c_str(),
			timeRemainingStr.c_str());

	if (_config.checkpointFile.length() > 0) {
		uint64_t t = CommonUtils::getSystemTime();
		if (t - _lastUpdate >= _config.checkpointInterval) {
			Logger::log(LogLevel::Info, "Checkpoint");
			writeCheckpoint(info.nextKey);
			_lastUpdate = t;
		}
	}
}

/**
 * Parses the start:end key pair. Possible values are:
 start
 start:end
 start:+offset
 :end
 :+offset
 */
bool parseKeyspace(const std::string &s, secp256k1::uint256 &start,
		secp256k1::uint256 &end);
bool parseKeyspace(const std::string &s, secp256k1::uint256 &start,
		secp256k1::uint256 &end) {
	size_t pos = s.find(':');

	if (pos == std::string::npos) {
		start = secp256k1::uint256(s);
		end = secp256k1::N - 1;
	} else {
		std::string left = s.substr(0, pos);

		if (left.length() == 0) {
			start = secp256k1::uint256(1);
		} else {
			start = secp256k1::uint256(left);
		}

		std::string right = s.substr(pos + 1);

		if (right[0] == '+') {
			end = start + secp256k1::uint256(right.substr(1));
		} else {
			end = secp256k1::uint256(right);
		}
	}

	return true;
}

void usage();
void usage() {
	printf("CleanedBitCrack OPTIONS [TARGETS]\n");
	printf("Where TARGETS is one or more addresses\n\n");

	printf("--help                  Display this message\n");
	printf("-c, --compressed        Use compressed points\n");
	printf("-u, --uncompressed      Use Uncompressed points\n");
	printf("-r, --random            Use random values from keyspace\n");
	printf("--compression  MODE     Specify compression where MODE is\n");
	printf("                          COMPRESSED or UNCOMPRESSED or BOTH\n");
	printf("-d, --device ID         Use device ID\n");
	printf("-b, --blocks N          N blocks\n");
	printf("-t, --threads N         N threads per block\n");
	printf("-p, --points N          N points per thread\n");
	printf("-i, --in FILE           Read addresses from FILE, one per line\n");
	printf("-o, --out FILE          Write keys to FILE\n");
	printf("-f, --follow            Follow text output\n");
	printf("--list-devices          List available devices\n");
	printf("--keyspace KEYSPACE     Specify the keyspace:\n");
	printf("                          START:END\n");
	printf("                          START:+COUNT\n");
	printf("                          START\n");
	printf("                          :END\n");
	printf("                          :+COUNT\n");
	printf(
			"                        Where START, END, COUNT are in hex format\n");
	printf("--stride N              Increment by N keys at a time\n");
	printf(
			"--share M/N             Divide the keyspace into N equal shares, process the Mth share\n");
	printf("--continue FILE         Save/load progress from FILE\n");
}

/**
 Finds default parameters depending on the device
 */
typedef struct DeviceParameters {
	unsigned int threads;
	unsigned int blocks;
	unsigned int pointsPerThread;
} DeviceParameters;

DeviceParameters getDefaultParameters(const DeviceManager::DeviceInfo &device);
DeviceParameters getDefaultParameters(const DeviceManager::DeviceInfo &device) {
	DeviceParameters parameters = {0,0,0};

#ifdef BUILD_CUDA
	if (device.type == DeviceManager::DeviceType::CUDA) {
		parameters.threads = 256;
	}
#endif

#ifdef BUILD_OPENCL
	if (device.type == DeviceManager::DeviceType::OpenCL) {
		size_t maxWorkingGroupUnrecomendedSize = 256;
		size_t maxWorkingGroupSizeDefaultSize = sizeof(device.maxWorkingGroupSize);
		if (maxWorkingGroupSizeDefaultSize == maxWorkingGroupUnrecomendedSize) {
			parameters.threads = device.maxWorkingGroupSize / 2;
		} else {
			parameters.threads = device.maxWorkingGroupSize;
		}
	}
#endif

	parameters.blocks = device.computeUnits * parameters.threads;
	parameters.pointsPerThread = pow(2, floor(log(device.memory / (32 * 1024 * parameters.blocks)) / log(2))); //best power of 2 based on device memory assuming 4kb per block.

	return parameters;
}

static KeySearchDevice *
getDeviceContext(DeviceManager::DeviceInfo &device, int blocks, int threads,
		int pointsPerThread) {
#ifdef BUILD_CUDA
    if (device.type == DeviceManager::DeviceType::CUDA) {
        return new CudaKeySearchDevice(static_cast<int>(device.physicalId), threads, pointsPerThread, blocks);
    }
#endif

#ifdef BUILD_OPENCL
    if (device.type == DeviceManager::DeviceType::OpenCL) {
        return new CLKeySearchDevice(device.physicalId, threads, pointsPerThread, blocks);
    }
#endif

	return nullptr;
}

static void printDeviceList(
		const std::vector<DeviceManager::DeviceInfo> &devices) {
	for (size_t i = 0; i < devices.size(); i++) {
		printf("ID:     %d\n", devices[i].id);
		printf("Name:   %s\n", devices[i].name.c_str());
		printf("Memory: %" PRIu64 "MB\n",
				devices[i].memory / static_cast<uint64_t>(1024 * 1024));
		printf("Compute units: %d\n", devices[i].computeUnits);
		printf("\n");
	}
}

bool readAddressesFromFile(const std::string &fileName,
		std::vector<std::string> &lines) {
	if (fileName == "-") {
		return CommonUtils::readLinesFromStream(std::cin, lines);
	} else {
		return CommonUtils::readLinesFromStream(fileName, lines);
	}
}

PointCompressionType::Value  parseCompressionString(const std::string &s) {
	std::string mode = CommonUtils::toLower(s);

	if (mode == "both") {
		return PointCompressionType::BOTH;
	}

	if (mode == "compressed") {
		return PointCompressionType::COMPRESSED;
	}

	if (mode == "uncompressed") {
		return PointCompressionType::UNCOMPRESSED;
	}

	throw std::string("Invalid compression format: '" + s + "'");
}

static std::string getCompressionString(int mode) {
	switch (mode) {
		case PointCompressionType::BOTH:
			return "both";
		case PointCompressionType::UNCOMPRESSED:
			return "uncompressed";
		case PointCompressionType::COMPRESSED:
			return "compressed";
	}

	throw std::string(
			"Invalid compression setting '" + CommonUtils::format(mode) + "'");
}

void writeCheckpoint(secp256k1::uint256 nextKey) {
	std::ofstream tmp(_config.checkpointFile, std::ios::out);

	tmp << "start=" << _config.startKey.toString() << std::endl;
	tmp << "next=" << nextKey.toString() << std::endl;
	tmp << "end=" << _config.endKey.toString() << std::endl;
	tmp << "blocks=" << _config.blocks << std::endl;
	tmp << "threads=" << _config.threads << std::endl;
	tmp << "points=" << _config.pointsPerThread << std::endl;
	tmp << "compression=" << getCompressionString(_config.compression)
			<< std::endl;
	tmp << "device=" << _config.device << std::endl;
	tmp << "elapsed="
			<< (_config.elapsed + CommonUtils::getSystemTime() - _startTime)
			<< std::endl;
	tmp << "stride=" << _config.stride.toString();
	tmp.close();
}

void readCheckpointFile() {
	if (_config.checkpointFile.length() == 0) {
		return;
	}

	ConfigFileReader reader(_config.checkpointFile);

	if (!reader.exists()) {
		return;
	}

	Logger::log(LogLevel::Info, "Loading ' " + _config.checkpointFile + "'");

	std::map<std::string, ConfigFileEntry> entries = reader.read();

	_config.startKey = secp256k1::uint256(entries["start"].value);
	_config.nextKey = secp256k1::uint256(entries["next"].value);
	_config.endKey = secp256k1::uint256(entries["end"].value);

	if (_config.threads == 0 && entries.find("threads") != entries.end()) {
		_config.threads = CommonUtils::parseUInt32(entries["threads"].value);
	}
	if (_config.blocks == 0 && entries.find("blocks") != entries.end()) {
		_config.blocks = CommonUtils::parseUInt32(entries["blocks"].value);
	}
	if (_config.pointsPerThread == 0
			&& entries.find("points") != entries.end()) {
		_config.pointsPerThread = CommonUtils::parseUInt32(
				entries["points"].value);
	}
	if (entries.find("compression") != entries.end()) {
		_config.compression = parseCompressionString(
				entries["compression"].value);
	}
	if (entries.find("elapsed") != entries.end()) {
		_config.elapsed = CommonUtils::parseUInt32(entries["elapsed"].value);
	}
	if (entries.find("stride") != entries.end()) {
		_config.stride = secp256k1::uint256(entries["stride"].value);
	}

	_config.totalkeys = (_config.nextKey - _config.startKey).toUint64();
}

int run() {
	Logger::log(LogLevel::Info, "CleanedBitCrack\n");

	if (_config.device < 0 || _config.device >= static_cast<int>(_devices.size())) {
		Logger::log(LogLevel::Error,
				"device " + CommonUtils::format(_config.device)
						+ " does not exist");
		return 1;
	}

	Logger::log(LogLevel::Info,
			"Compression: " + getCompressionString(_config.compression));
	Logger::log(LogLevel::Info, "Starting at: " + _config.nextKey.toString());
	Logger::log(LogLevel::Info, "Ending at:   " + _config.endKey.toString());

	if (_config.randomMode) {
		Logger::log(LogLevel::Info, "Generating random starting points");
	}

	Logger::log(LogLevel::Info, "Counting by: " + _config.stride.toString());

	try {
		_lastUpdate = CommonUtils::getSystemTime();
		_startTime = CommonUtils::getSystemTime();

		// Use default parameters if they have not been set
		DeviceParameters params = getDefaultParameters(
				_devices[_config.device]);

		if (_config.blocks == 0) {
			_config.blocks = params.blocks;
		}

		if (_config.threads == 0) {
			_config.threads = params.threads;
		}

		if (_config.pointsPerThread == 0) {
			_config.pointsPerThread = params.pointsPerThread;
		}

		Logger::log(LogLevel::Info, "Threads: " + std::to_string(_config.threads));
		Logger::log(LogLevel::Info, "Blocks: " + std::to_string(_config.blocks));
		Logger::log(LogLevel::Info, "Points per Thread: " + std::to_string(_config.pointsPerThread));

		// Get device context
		KeySearchDevice *d = getDeviceContext(_devices[_config.device],
				_config.blocks, _config.threads, _config.pointsPerThread);

		KeyFinder f(_config.nextKey, _config.endKey, _config.compression, d,
				_config.stride, _config.randomMode);

		f.setResultCallback(resultCallback);
		f.setStatusInterval(_config.statusInterval);
		f.setStatusCallback(statusCallback);

		f.init();

		if (!_config.targetsFile.empty()) {
			f.setTargets(_config.targetsFile);
		} else {
			f.setTargets(_config.targets);
		}

		f.run();

		delete d;
	} catch (KeySearchException &ex) {
		Logger::log(LogLevel::Info, ex.msg + ": " + ex.description);
		return 1;
	}

	return 0;
}

/**
 * Parses a string in the form of x/y
 */
bool parseShare(const std::string &s, uint32_t &idx, uint32_t &total) {
	size_t pos = s.find('/');
	if (pos == std::string::npos) {
		return false;
	}

	try {
		idx = CommonUtils::parseUInt32(s.substr(0, pos));
	} catch (...) {
		return false;
	}

	try {
		total = CommonUtils::parseUInt32(s.substr(pos + 1));
	} catch (...) {
		return false;
	}

	if (idx == 0 || total == 0) {
		return false;
	}

	if (idx > total) {
		return false;
	}

	return true;
}

int main(int argc, char **argv) {
	bool optCompressed = false;
	bool optUncompressed = false;
	bool listDevices = false;
	bool optShares = false;
	bool optContinue = false;

	uint32_t shareIdx = 0;
	uint32_t numShares = 0;

	// Catch --help first and Check for arguments
	for (int i = 1; i < argc; i++)
	{
		if(
			std::string(argv[i]) == "--help" ||
			std::string(argv[i]) == "-h" ||
			std::string(argv[i]) == "-?" ||
			argc == 1
		)
		{
			usage();
			return 0;
		}
	}

	// Check for supported devices
	try {
		_devices = DeviceManager::getDevices();

		if (_devices.size() == 0) {
			Logger::log(LogLevel::Error, "No devices available");
			return 1;
		}
	} catch (DeviceManager::DeviceManagerException &ex) {
		Logger::log(LogLevel::Error, "Error detecting devices: " + ex.msg);
		return 1;
	}

	CmdParse parser;
	parser.add("-d", "--device", true);
	parser.add("-t", "--threads", true);
	parser.add("-b", "--blocks", true);
	parser.add("-p", "--points", true);
	parser.add("-d", "--device", true);
	parser.add("-c", "--compressed", false);
	parser.add("-u", "--uncompressed", false);
	parser.add("-r", "--random", false);
	parser.add("", "--compression", true);
	parser.add("-i", "--in", true);
	parser.add("-o", "--out", true);
	parser.add("-f", "--follow", false);
	parser.add("-l", "--list-devices", false);
	parser.add("-k", "--keyspace", true);
	parser.add("", "--continue", true);
	parser.add("", "--share", true);
	parser.add("", "--stride", true);

	try {
		parser.parse(argc, argv);
	} catch (std::string &err) {
		Logger::log(LogLevel::Error, "Error: " + err);
		return 1;
	}

	std::vector<OptArg> args = parser.getArgs();

	for (unsigned int i = 0; i < args.size(); i++) {
		OptArg optArg = args[i];
		std::string opt = args[i].option;

		try {
			if (optArg.equals("-t", "--threads")) {
				_config.threads = CommonUtils::parseUInt32(optArg.arg);
			} else if (optArg.equals("-b", "--blocks")) {
				_config.blocks = CommonUtils::parseUInt32(optArg.arg);
			} else if (optArg.equals("-p", "--points")) {
				_config.pointsPerThread = CommonUtils::parseUInt32(optArg.arg);
			} else if (optArg.equals("-d", "--device")) {
				_config.device = CommonUtils::parseUInt32(optArg.arg);
			} else if (optArg.equals("-c", "--compressed")) {
				optCompressed = true;
			} else if (optArg.equals("-u", "--uncompressed")) {
				optUncompressed = true;
			} else if (optArg.equals("-r", "--random")) {
				_config.randomMode = true;
			} else if (optArg.equals("", "--compression")) {
				_config.compression = parseCompressionString(optArg.arg);
			} else if (optArg.equals("-i", "--in")) {
				_config.targetsFile = optArg.arg;
			} else if (optArg.equals("-o", "--out")) {
				_config.resultsFile = optArg.arg;
			} else if (optArg.equals("-l", "--list-devices")) {
				listDevices = true;
			} else if (optArg.equals("", "--continue")) {
				optContinue = true;
				_config.checkpointFile = optArg.arg;
			} else if (optArg.equals("-k", "--keyspace")) {
				secp256k1::uint256 start;
				secp256k1::uint256 end;

				parseKeyspace(optArg.arg, start, end);

				if (start.cmp(secp256k1::N) > 0) {
					throw std::string("argument is out of range");
				}
				if (start.isZero()) {
					throw std::string("argument is out of range");
				}
				if (end.cmp(secp256k1::N) > 0) {
					throw std::string("argument is out of range");
				}
				if (start.cmp(end) > 0) {
					throw std::string("Invalid argument");
				}

				_config.startKey = start;
				_config.nextKey = start;
				_config.endKey = end;
			} else if (optArg.equals("", "--share")) {
				if (!parseShare(optArg.arg, shareIdx, numShares)) {
					throw std::string("Invalid argument");
				}
				optShares = true;
			} else if (optArg.equals("", "--stride")) {
				try {
					_config.stride = secp256k1::uint256(optArg.arg);
				} catch (...) {
					throw std::string(
							"invalid argument: : expected hex string");
				}
				if (_config.stride.cmp(secp256k1::N) >= 0) {
					throw std::string("argument is out of range");
				}
				if (_config.stride.cmp(0) == 0) {
					throw std::string("argument is out of range");
				}
			} else if (optArg.equals("-f", "--follow")) {
				_config.follow = true;
			}
		} catch (std::string &err) {
			Logger::log(LogLevel::Error, "Error " + opt + ": " + err);
			return 1;
		}
	}

	if (optContinue && _config.randomMode) {
		Logger::log(LogLevel::Error,
				"Random and continue mode cannot be used together.");
		return 1;
	}

	if (listDevices) {
		printDeviceList(_devices);
		return 0;
	}

	// Verify device exists
	if (_config.device < 0 || _config.device >= static_cast<int>(_devices.size())) {
		Logger::log(LogLevel::Error,
				"device " + CommonUtils::format(_config.device)
						+ " does not exist");
		return 1;
	}

	// Parse operands
	std::vector<std::string> ops = parser.getOperands();

	// If there are no operands, then we must be reading from a file, otherwise
	// expect addresses on the command-line
	if (ops.size() == 0) {
		if (_config.targetsFile.length() == 0) {
			Logger::log(LogLevel::Error, "Missing arguments");
			usage();
			return 1;
		}
	} else {
		for (size_t i = 0; i < ops.size(); i++) {
			if (!Address::verifyAddress(ops[i])) {
				Logger::log(LogLevel::Error,
						"Invalid address '" + ops[i] + "'");
				return 1;
			}
			_config.targets.push_back(ops[i]);
		}
	}

	// Calculate where to start and end in the keyspace when the --share option is used
	if (optShares) {
		Logger::log(LogLevel::Info,
				"Share " + CommonUtils::format(shareIdx) + " of "
						+ CommonUtils::format(numShares));
		secp256k1::uint256 numKeys = _config.endKey - _config.nextKey + 1;

		secp256k1::uint256 diff = numKeys.mod(numShares);
		numKeys = numKeys - diff;

		secp256k1::uint256 shareSize = numKeys.div(numShares);

		secp256k1::uint256 startPos = _config.nextKey
				+ (shareSize * (shareIdx - 1));

		if (shareIdx < numShares) {
			secp256k1::uint256 endPos = _config.nextKey
					+ (shareSize * (shareIdx)) - 1;
			_config.endKey = endPos;
		}

		_config.nextKey = startPos;
		_config.startKey = startPos;
	}

	// Check option for compressed, uncompressed, or both
	if (optCompressed && optUncompressed) {
		_config.compression = PointCompressionType::BOTH;
	} else if (optCompressed) {
		_config.compression = PointCompressionType::COMPRESSED;
	} else if (optUncompressed) {
		_config.compression = PointCompressionType::UNCOMPRESSED;
	}

	if (_config.checkpointFile.length() != 0) {
		readCheckpointFile();
	}

	return run();
}
