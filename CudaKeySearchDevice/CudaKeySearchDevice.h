#ifndef _CUDA_KEY_SEARCH_DEVICE
#define _CUDA_KEY_SEARCH_DEVICE

#include "CudaAtomicList.h"
#include "CudaDeviceKeys.h"
#include "CudaHashLookup.h"
#include "KeySearchDevice.h"
#include "cudaUtil.h"
#include "secp256k1.h"
#include <cuda_runtime.h>
#include <vector>

// Structures that exist on both host and device side
struct CudaDeviceResult {
	int thread;
	int block;
	int idx;
	bool compressed;
	unsigned int x[8];
	unsigned int y[8];
	unsigned int digest[5];
};

class CudaKeySearchDevice: public KeySearchDevice {
private:
	int _device;

	int _blocks = 0;

	int _threads;

	int _pointsPerThread;

	int _compression = PointCompressionType::COMPRESSED;

	secp256k1::uint256 _start;

	secp256k1::uint256 _end;

	secp256k1::uint256 _stride;

	secp256k1::uint256 _startExponent;

	std::vector<KeySearchResult> _results;

	std::vector<hash160> _targets;

	std::string _deviceName;

	CudaDeviceKeys _deviceKeys;

	CudaAtomicList _resultList;

	CudaHashLookup _targetLookup;

	void cudaCall(cudaError_t err);

	void generateStartingPoints();

	void getResultsInternal();

	void removeTargetFromList(const unsigned int hash[5]);

	uint32_t getPrivateKeyOffset(int thread, int block, int point);

	uint64_t _iterations;

	bool _randomMode = false;

	bool isTargetInList(const unsigned int hash[5]);

	bool
	verifyKey(const secp256k1::uint256 &privateKey,
			const secp256k1::ecpoint &publicKey, const unsigned int hash[5],
			bool compressed);

public:
	CudaKeySearchDevice(int device, int threads,
		int pointsPerThread, int blocks = 0, int compression = PointCompressionType::COMPRESSED);
	virtual ~CudaKeySearchDevice();

	// Initialize the device
	virtual void init(const secp256k1::uint256& start,
                      const secp256k1::uint256& end,
                      int compression,
                      const secp256k1::uint256& stride,
                      bool randomMode);

	virtual void doStep();

	virtual void setTargets(const std::set<KeySearchTarget>& targets);

	virtual size_t getResults(std::vector<KeySearchResult>& results);

	virtual uint64_t keysPerStep();

	virtual std::string getDeviceName();

	virtual void getMemoryInfo(uint64_t& freeMem, uint64_t& totalMem);

	virtual secp256k1::uint256 getNextKey();
};

#endif
