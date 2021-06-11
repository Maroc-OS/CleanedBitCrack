#include "clContext.h"
#include <iostream>
#include <vector>

#define SECTION_ADD 0
#define SECTION_MULTIPLY 1
#define SECTION_INVERSE 2

static std::string _sections[] = { "Addition", "Multiplication", "Inverse" };

typedef struct {
	int section;
} CLErrorInfo;

extern char _secp256k1_test_cl[];

int runTest(cl_device_id deviceId);
int runTest(cl_device_id deviceId) {
	cl::CLContext ctx(deviceId);
	cl::CLProgram prog(ctx, _secp256k1_test_cl);
	cl::CLKernel k(prog, "secp256k1_test");

	cl_mem devNumErrors = ctx.malloc(sizeof(size_t));
	cl_mem devErrors = ctx.malloc(sizeof(CLErrorInfo) * 1000);

	std::cout << "Running test kernel..." << std::endl;

	k.set_args(devErrors, devNumErrors);
	k.call(1, 1);

	int numErrors = 0;
	std::vector<CLErrorInfo> errors;

	ctx.copyDeviceToHost(devNumErrors, &numErrors, sizeof(size_t));

	std::cout << numErrors << " errors" << std::endl;

	if (numErrors > 0) {
		errors.resize(static_cast<size_t>(numErrors));

		ctx.copyDeviceToHost(devErrors, errors.data(),
				sizeof(CLErrorInfo) * static_cast<size_t>(numErrors));

		for (int i = 0; i < numErrors; i++) {
			std::cout << _sections[errors[static_cast<size_t>(i)].section] << " test failed"
					<< std::endl;
		}
	}

	ctx.free(devNumErrors);
	ctx.free(devErrors);

	return numErrors;
}

int main(int /* unused */, char **/* unused */) {
	std::vector<cl::CLDeviceInfo> devices;

	try {
		devices = cl::getDevices();
	} catch (cl::CLException &ex) {
		std::cout << "Error: " << ex.msg << std::endl;
		return 1;
	}

	std::cout << "Found " << devices.size() << " devices" << std::endl;

	if (devices.size() == 0) {
		std::cout << "No OpenCL devices found" << std::endl;
		return 0;
	}

	int numErrors = 0;

	for (size_t i = 0; i < devices.size(); i++) {
		try {
			std::cout << "Testing device " << devices[i].name << std::endl;
			numErrors += runTest(devices[i].id);
		} catch (cl::CLException &ex) {
			std::cout << "Error " << ex.msg << std::endl;
		}
	}

	std::cout << std::endl;

	if (!numErrors) {
		std::cout << "PASS" << std::endl;
	} else {
		std::cout << "FAIL" << std::endl;
	}

	return 0;
}
