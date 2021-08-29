#include <fstream>
#include <sstream>

#include "clContext.h"
#include "CommonUtils.h"

cl::CLContext::CLContext(cl_device_id device) : _device(device) {
	cl_int errorCode = CL_SUCCESS;
	_ctx = clCreateContext(nullptr, 1, &_device, nullptr, nullptr, &errorCode);
	clCall(errorCode);

	_queue = clCreateCommandQueue(_ctx, _device, 0, &errorCode);
	clCall(errorCode);
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
	cl_int errorCode = CL_SUCCESS;
	cl_mem ptr = clCreateBuffer(_ctx, flags, size, nullptr, &errorCode);
	clCall(errorCode);
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
	clCall(
			clEnqueueFillBuffer(_queue, devicePtr, &value, sizeof(unsigned char),
					0, size, 0, nullptr, nullptr));
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
	cl_int errorCode = CL_SUCCESS;

	options += " -cl-kernel-arg-info -cl-std=CL1.2 -cl-mad-enable -cl-no-signed-zeros -cl-unsafe-math-optimizations";

	if (CommonUtils::toLower(_ctx.getDeviceVendor()).find("intel")
			!= std::string::npos) {
		options += " -DDEVICE_VENDOR_INTEL";
	}

	_prog = clCreateProgramWithSource(ctx.getContext(), 1, &ptr, &len, &errorCode);
	clCall(errorCode);

    errorCode = clBuildProgram(_prog, 0, nullptr, options.c_str(), nullptr, nullptr);

    if ((errorCode == CL_BUILD_PROGRAM_FAILURE) || (errorCode != CL_SUCCESS)) {
		size_t logSize = 0;
		clGetProgramBuildInfo(_prog, ctx.getDevice(), CL_PROGRAM_BUILD_LOG, 0,
				nullptr, &logSize);

		char *log = new char[logSize];
		clGetProgramBuildInfo(_prog, ctx.getDevice(), CL_PROGRAM_BUILD_LOG,
				logSize, log, nullptr);

		_buildLog = std::string(log, logSize);
		delete[] log;

		throw CLException(errorCode, _buildLog);
	}
	clCall(errorCode);
}

cl::CLProgram::CLProgram(cl::CLContext &ctx, const char *src,
		std::string options) :
		_ctx(ctx) {
	size_t len = strlen(src);
	cl_int errorCode = CL_SUCCESS;

	options += " -cl-kernel-arg-info -cl-std=CL1.2 -cl-mad-enable -cl-no-signed-zeros -cl-unsafe-math-optimizations";

	if (CommonUtils::toLower(_ctx.getDeviceVendor()).find("intel")
			!= std::string::npos) {
		options += " -DDEVICE_VENDOR_INTEL";
	}

	_prog = clCreateProgramWithSource(ctx.getContext(), 1, &src, &len, &errorCode);
	clCall(errorCode);

	errorCode = clBuildProgram(_prog, 0, nullptr, options.c_str(), nullptr, nullptr);

	if ((errorCode == CL_BUILD_PROGRAM_FAILURE) || (errorCode != CL_SUCCESS)) {
		size_t logSize = 0;
		clGetProgramBuildInfo(_prog, ctx.getDevice(), CL_PROGRAM_BUILD_LOG, 0,
				nullptr, &logSize);

		char *log = new char[logSize];
		clGetProgramBuildInfo(_prog, ctx.getDevice(), CL_PROGRAM_BUILD_LOG,
				logSize, log, nullptr);

		_buildLog = std::string(log, logSize);
		delete[] log;

		throw CLException(errorCode, _buildLog);
	}
	clCall(errorCode);
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

int cl::CLContext::get_mp_count()
{
	size_t count = 1;

	clCall(clGetDeviceInfo(_device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(count), &count, nullptr));

	return (int)count;
}

// TODO: This is for 1 dimension only
int cl::CLContext::get_max_block_size()
{
	size_t count[3] = { 1,1,1 };
	size_t max_items = 1;

	clCall(clGetDeviceInfo(_device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(count), &count, nullptr));

	clCall(clGetDeviceInfo(_device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_items), &max_items, nullptr));

	return (int)std::min(count[0], max_items);
}

cl::CLProgram::~CLProgram() {
	clReleaseProgram(_prog);
}

cl::CLKernel::CLKernel(cl::CLProgram &prog, std::string entry) :
		_prog(prog), _entry(entry) {
	const char *ptr = entry.c_str();
	cl_int errorCode = CL_SUCCESS;
	_kernel = clCreateKernel(_prog.getProgram(), ptr, &errorCode);
	clCall(errorCode);
}

size_t cl::CLKernel::getWorkGroupSize() {
	size_t size = 0;

	cl_int errorCode = clGetKernelWorkGroupInfo(_kernel,
			_prog.getContext().getDevice(), CL_KERNEL_WORK_GROUP_SIZE,
			sizeof(size_t), &size, nullptr);

	clCall(errorCode);

	return size;
}

cl::CLKernel::~CLKernel() {
	clReleaseKernel(_kernel);
}
