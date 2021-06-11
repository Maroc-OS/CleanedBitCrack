#ifndef _DEVICE_MANAGER_H
#define _DEVICE_MANAGER_H

#include <cstdint>
#include <iostream>
#include <vector>

namespace DeviceManager {

class DeviceManagerException {
public:
    std::string msg;

    explicit DeviceManagerException(const std::string& dm_msg) : msg(dm_msg) {
	}
};

class DeviceType {
public:
	enum {
		CUDA = 0, OpenCL
	};
};

typedef struct DeviceInfo {
	int type;
	int id;

	// General device info
	uint64_t physicalId;
	std::string name;
	uint64_t memory;
	int computeUnits;
	int maxWorkingGroupSize;

	// CUDA device info
	int cudaMajor;
	int cudaMinor;
	int cudaCores;
} DeviceInfo;

std::vector<DeviceInfo> getDevices();

}

#endif
