#include "cudabridge.h"


__global__ void _stepKernel(int points, int compression);
__global__ void _stepKernelWithDouble(int points, int compression);

void callKeyFinderKernel(int blocks, int threads, int points, bool useDouble, int compression)
{
    if (useDouble) {
        _stepKernelWithDouble<<<blocks, threads>>>(points, compression);
    } else {
        _stepKernel<<<blocks, threads>>>(points, compression);
    }
    waitForKernel();
}


void waitForKernel()
{
    // Check for kernel launch error
    cudaError_t err = cudaGetLastError();

    if (err != cudaSuccess) {
        throw cuda::CudaException(err);
    }

    // Wait for kernel to complete
    err = cudaDeviceSynchronize();
    fflush(stdout);
    if (err != cudaSuccess) {
        throw cuda::CudaException(err);
    }
}