#ifndef _CL_UTIL_H
#define _CL_UTIL_H

#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#if defined(__APPLE__)
#define CL_SILENCE_DEPRECATION

#ifdef __cplusplus
extern "C" {
#endif
#include <OpenCL/opencl.h>
#ifdef __cplusplus
}
#endif

#elif defined(__linux__) || defined(__CYGWIN__)
#include <CL/cl.h>
#else
#ifdef __cplusplus
extern "C" {
#endif
#define NVCL_SUPPRESS_USE_DEPRECATED_OPENCL_1_0_APIS_WARNING
#include <CL/cl.h>
#ifdef __cplusplus
}
#endif
#endif

#include <string>
#include <vector>

namespace cl {
std::string getOpenCLErrorName(cl_int errorCode);
std::string getOpenCLErrorDescription(cl_int errorCode);
typedef struct CLDeviceInfo {
	cl_device_id id;
	int cores;
	uint64_t mem;
	std::string name;
	size_t maxWorkingGroupSize;
} CLDeviceInfo;

class CLException {
public:
	int error;
	std::string msg;
	std::string description;

	explicit CLException(cl_int errorCode) : error(errorCode), msg(getOpenCLErrorName(errorCode)), description(getOpenCLErrorDescription(errorCode)) {
	}

	CLException(cl_int errorCode, const std::string &cl_msg) : error(errorCode), msg(cl_msg), description(getOpenCLErrorDescription(errorCode)){
	}

	CLException(cl_int errorCode, const std::string &cl_msg, const std::string &cl_description) : error(errorCode), msg(cl_msg), description(cl_description){
	}
};

CLDeviceInfo getDeviceInfo(int device);

std::vector<CLDeviceInfo> getDevices();

int getDeviceCount();

void clCall(cl_int errorCode);

}

#endif
