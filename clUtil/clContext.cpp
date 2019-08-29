#include <fstream>
#include <sstream>

#include "clContext.h"
#include "CommonUtils.h"

cl::CLContext::CLContext(cl_device_id device) : _device(device) {
	cl_int err = 0;
	_ctx = clCreateContext(nullptr, 1, &_device, nullptr, nullptr, &err);
	clCall(err);

	_queue = clCreateCommandQueue(_ctx, _device, 0, &err);
	clCall(err);
}

cl::CLContext::~CLContext() {
	clReleaseCommandQueue(_queue);
	clReleaseContext(_ctx);
}

cl_device_id cl::CLContext::getDevice() {
	return _device;
}

cl_command_queue cl::CLContext::getQueue() {
	return _queue;
}

cl_context cl::CLContext::getContext() {
	return _ctx;
}

cl_mem cl::CLContext::malloc(size_t size, cl_mem_flags flags) {
	cl_int err = 0;
	cl_mem ptr = clCreateBuffer(_ctx, flags, size, nullptr, &err);
	clCall(err);
	this->memset(ptr, 0, size);
	return ptr;
}

void cl::CLContext::free(cl_mem mem) {
	clReleaseMemObject(mem);
}

void cl::CLContext::copyHostToDevice(const void *hostPtr, cl_mem devicePtr,
		size_t size) {
	clCall(
			clEnqueueWriteBuffer(_queue, devicePtr, CL_TRUE, 0, size, hostPtr,
					0, nullptr, nullptr));
}

void cl::CLContext::copyHostToDevice(const void *hostPtr, cl_mem devicePtr,
		size_t offset, size_t size) {
	clCall(
			clEnqueueWriteBuffer(_queue, devicePtr, CL_TRUE, offset, size,
					hostPtr, 0, nullptr, nullptr));
}

void cl::CLContext::copyDeviceToHost(cl_mem devicePtr, void *hostPtr,
		size_t size) {
	clCall(
			clEnqueueReadBuffer(_queue, devicePtr, CL_TRUE, 0, size, hostPtr, 0,
					nullptr, nullptr));
}

void cl::CLContext::memset(cl_mem devicePtr, unsigned char value, size_t size) {
#if CL_TARGET_OPENCL_VERSION >= 120
    clCall(clEnqueueFillBuffer(_queue, devicePtr, &value, sizeof(unsigned char), 0, size, 0, nullptr, nullptr));
#else
	unsigned char *ptr = new unsigned char[size];
	std::memset(ptr, value, size);
	copyHostToDevice(ptr, devicePtr, 0, size);
	delete[] ptr;
#endif
}

cl::CLProgram::CLProgram(cl::CLContext &ctx, std::string &src,
		std::string options) :
		_ctx(ctx) {
	std::string srcFile = loadSource(src);
	const char *ptr = srcFile.c_str();
	const size_t len = srcFile.length();
	cl_int err = CL_SUCCESS;

	if (CommonUtils::toLower(_ctx.getDeviceVendor()).find("intel")
			!= std::string::npos) {
            options += "-DDEVICE_VENDOR_INTEL -cl-std=CL2.0 -cl-mad-enable -cl-no-signed-zeros -cl-unsafe-math-optimizations";
        }

	_prog = clCreateProgramWithSource(ctx.getContext(), 1, &ptr, &len, &err);
	clCall(err);

    err = clBuildProgram(_prog, 0, nullptr, options.c_str(), nullptr, nullptr);

    if ((err == CL_BUILD_PROGRAM_FAILURE) || (err != CL_SUCCESS)) {
		size_t logSize = 0;
		clGetProgramBuildInfo(_prog, ctx.getDevice(), CL_PROGRAM_BUILD_LOG, 0,
				nullptr, &logSize);

		char *log = new char[logSize];
		clGetProgramBuildInfo(_prog, ctx.getDevice(), CL_PROGRAM_BUILD_LOG,
				logSize, log, nullptr);

		_buildLog = std::string(log, logSize);
		delete[] log;

		throw CLException(err, _buildLog);
	}
	clCall(err);
}

cl::CLProgram::CLProgram(cl::CLContext &ctx, const char *src,
		std::string options) :
		_ctx(ctx) {
	size_t len = strlen(src);
	cl_int err = CL_SUCCESS;

	if (CommonUtils::toLower(_ctx.getDeviceVendor()).find("intel")
			!= std::string::npos) {
            options += "-DDEVICE_VENDOR_INTEL -cl-std=CL2.0 -cl-mad-enable -cl-no-signed-zeros -cl-unsafe-math-optimizations";
        }

	_prog = clCreateProgramWithSource(ctx.getContext(), 1, &src, &len, &err);
	clCall(err);

	err = clBuildProgram(_prog, 0, nullptr, options.c_str(), nullptr, nullptr);

	if ((err == CL_BUILD_PROGRAM_FAILURE) || (err != CL_SUCCESS)) {
		size_t logSize = 0;
		clGetProgramBuildInfo(_prog, ctx.getDevice(), CL_PROGRAM_BUILD_LOG, 0,
				nullptr, &logSize);

		char *log = new char[logSize];
		clGetProgramBuildInfo(_prog, ctx.getDevice(), CL_PROGRAM_BUILD_LOG,
				logSize, log, nullptr);

		_buildLog = std::string(log, logSize);
		delete[] log;

		throw CLException(err, _buildLog);
	}
	clCall(err);
}

std::string cl::CLProgram::loadSource(const std::string &srcFile) {
	std::ifstream f(srcFile);
	if (!f.good()) {
		throw CLException(CL_BUILD_PROGRAM_FAILURE,
				"'" + srcFile + "' not found");
	}

	std::stringstream buf;
	buf << f.rdbuf();

	return buf.str();
}

cl_program cl::CLProgram::getProgram() {
	return _prog;
}

cl::CLContext &cl::CLProgram::getContext() {
	return _ctx;
}

uint64_t cl::CLContext::getGlobalMemorySize() {
	cl_ulong mem = 0;
	clCall(
			clGetDeviceInfo(_device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem),
					&mem, nullptr));

	return mem;
}

std::string cl::CLContext::getDeviceName() {
	char name[128] = { 0 };

	clCall(clGetDeviceInfo(_device, CL_DEVICE_NAME, sizeof(name), name, nullptr));

	return std::string(name);
}

std::string cl::CLContext::getDeviceVendor() {
	char name[128] = { 0 };

	clCall(
			clGetDeviceInfo(_device, CL_DEVICE_VENDOR, sizeof(name), name,
					nullptr));

	return std::string(name);
}

cl::CLProgram::~CLProgram() {
	clReleaseProgram(_prog);
}

cl::CLKernel::CLKernel(cl::CLProgram &prog, std::string entry) :
		_prog(prog), _entry(entry) {
	const char *ptr = entry.c_str();
	cl_int err = 0;
	_kernel = clCreateKernel(_prog.getProgram(), ptr, &err);
	clCall(err);
}

size_t cl::CLKernel::getWorkGroupSize() {
	size_t size = 0;

	cl_int err = clGetKernelWorkGroupInfo(_kernel,
			_prog.getContext().getDevice(), CL_KERNEL_WORK_GROUP_SIZE,
			sizeof(size_t), &size, nullptr);

	clCall(err);

	return size;
}

cl::CLKernel::~CLKernel() {
	clReleaseKernel(_kernel);
}
