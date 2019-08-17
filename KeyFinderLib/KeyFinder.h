#ifndef _KEY_FINDER_H
#define _KEY_FINDER_H

#include "KeySearchDevice.h"

class KeyFinder {
private:
	KeySearchDevice *_device;

	bool _randomMode = false;
	int _compression;

	std::set<KeySearchTarget> _targets;

	uint64_t _statusInterval;
	uint64_t _iterCount;
	uint64_t _total;
	uint64_t _totalTime;

	secp256k1::uint256 _startKey;
	secp256k1::uint256 _endKey;
	secp256k1::uint256 _stride = secp256k1::uint256(1);

	// Each index of each thread gets a flag to indicate if it found a valid hash
	bool _running;

	void (*_resultCallback)(KeySearchResult);

	void (*_statusCallback)(KeySearchStatus);

	static void defaultResultCallback(const KeySearchResult &result);

	static void defaultStatusCallback(const KeySearchStatus &status);

	void removeTargetFromList(const unsigned int value[5]);

	bool isTargetInList(const unsigned int value[5]);

	void setTargetsOnDevice();

public:
	KeyFinder(const secp256k1::uint256 &startKey,
			const secp256k1::uint256 &endKey, int compression,
			KeySearchDevice *device, const secp256k1::uint256 &stride,
			bool randomMode);

	~KeyFinder();

	void init();

	void run();

	void stop();

	void setResultCallback(void (*callback)(KeySearchResult));

	void setStatusCallback(void (*callback)(KeySearchStatus));

	void setStatusInterval(uint64_t interval);

	void setTargets(std::string targetFile);

	void setTargets(std::vector<std::string> &targets);

	secp256k1::uint256 getNextKey();
};

#endif
