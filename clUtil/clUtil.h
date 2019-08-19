#ifndef _CL_UTIL_H
#define _CL_UTIL_H

#ifdef __APPLE__
#define CL_SILENCE_DEPRECATION
#include <OpenCL/opencl.h>
#else

#include <CL/cl.h>

#endif

#include <string>
#include <vector>

namespace cl {
std::string getErrorString(cl_int err);

typedef struct {
	cl_device_id id;
	int cores;
	uint64_t mem;
	std::string name;
} CLDeviceInfo;

class CLException {
public:
	int error;
	std::string msg;

	explicit CLException(cl_int err) : error(err), msg(getErrorString(err)) {
	}

	CLException(cl_int err, const std::string &cl_msg) : error(err), msg(cl_msg) {
	}
};

CLDeviceInfo getDeviceInfo(int device);

std::vector<CLDeviceInfo> getDevices();

int getDeviceCount();

void clCall(cl_int err);

}

#endif
