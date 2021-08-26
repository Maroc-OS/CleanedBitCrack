#include "cudaUtil.h"

cuda::CudaDeviceInfo cuda::getDeviceInfo(int device) {
	cuda::CudaDeviceInfo devInfo;

	cudaDeviceProp properties;
	cudaError_t err = cudaSuccess;

	err = cudaSetDevice(device);

	if (err) {
		throw cuda::CudaException(err);
	}

	err = cudaGetDeviceProperties(&properties, device);

	if (err) {
		throw cuda::CudaException(err);
	}

	devInfo.id = device;
	devInfo.major = properties.major;
	devInfo.minor = properties.minor;
	devInfo.mpCount = properties.multiProcessorCount;
	devInfo.mem = properties.totalGlobalMem;
	devInfo.name = std::string(properties.name);

	int cores = 0;
	switch (devInfo.major) {
	case 1:
		cores = 8;
		break;
	case 2:
		if (devInfo.minor == 0) {
			cores = 32;
		} else {
			cores = 48;
		}
		break;
	case 3: // "Kepler"
		cores = 192;
		break;
	case 5: // "Maxwell"
		cores = 128;
		break;
	case 6: // "Pascal"
		if (devInfo.minor == 1 || devInfo.minor == 2) {
			cores = 128;
		} else {
			cores = 64;
		}
		break;
	case 7: // "Volta" & "Xavier" & "Turing"
		cores = 64;
		break;
	case 8: // "Ampere"
		if (devInfo.minor == 6) {
			cores = 128;
		} else {
			cores = 64;
		}
		break;
	default:
		cores = 8;
		break;
	}
	devInfo.cores = cores;

	return devInfo;
}

std::vector<cuda::CudaDeviceInfo> cuda::getDevices() {
	int count = getDeviceCount();

	std::vector<CudaDeviceInfo> devList;

	for (int device = 0; device < count; device++) {
		devList.push_back(getDeviceInfo(device));
	}

	return devList;
}

int cuda::getDeviceCount() {
	int count = 0;

	cudaError_t err = cudaGetDeviceCount(&count);

	if (err) {
		throw cuda::CudaException(err);
	}

	return count;
}
