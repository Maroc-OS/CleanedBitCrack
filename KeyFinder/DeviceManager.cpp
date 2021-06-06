#include "DeviceManager.h"

#ifdef BUILD_CUDA
#include "cudaUtil.h"
#endif

#ifdef BUILD_OPENCL
#include "clUtil.h"
#endif

std::vector<DeviceManager::DeviceInfo> DeviceManager::getDevices() {
	std::vector<DeviceManager::DeviceInfo> devices;

#ifdef BUILD_CUDA
    // Get CUDA devices
    try {
        std::vector<cuda::CudaDeviceInfo> cudaDevices = cuda::getDevices();

        for (size_t deviceId = 0; deviceId < cudaDevices.size(); deviceId++) {
            DeviceManager::DeviceInfo device;
            device.name = cudaDevices[deviceId].name;
            device.type = DeviceType::CUDA;
            device.id = deviceId;
            device.physicalId = cudaDevices[deviceId].id;
            device.memory = cudaDevices[deviceId].mem;
            device.computeUnits = cudaDevices[deviceId].mpCount;
            devices.push_back(device);
        }
    } catch (cuda::CudaException &ex) {
        throw DeviceManager::DeviceManagerException(ex.msg);
    }
#endif

#ifdef BUILD_OPENCL
    // Get OpenCL devices
    try {
        std::vector<cl::CLDeviceInfo> clDevices = cl::getDevices();

        for (size_t deviceId = 0; deviceId < clDevices.size(); deviceId++) {
            DeviceManager::DeviceInfo device;
            device.name = clDevices[deviceId].name;
            device.type = DeviceType::OpenCL;
            device.id = deviceId;
            device.physicalId = (uint64_t)clDevices[deviceId].id;
            device.memory = clDevices[deviceId].mem;
            device.computeUnits = clDevices[deviceId].cores;
            device.maxWorkingGroupSize = clDevices[deviceId].maxWorkingGroupSize;
            devices.push_back(device);
        }
    } catch (cl::CLException &ex) {
        throw DeviceManager::DeviceManagerException(ex.msg);
    }
#endif

	return devices;
}
