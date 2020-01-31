#ifndef _ATOMIC_LIST_HOST_H
#define _ATOMIC_LIST_HOST_H

#include <cuda_runtime.h>
#if defined(__APPLE__)
#include <sys/_types/_null.h>
#elif defined(__linux__) || defined(__CYGWIN__)
#include <sys/types.h>
#endif

/**
 A list that multiple device threads can append items to. Items can be
 read and removed by the host
 */
class CudaAtomicList
{
private:
    void* _devPtr;

    void* _hostPtr;

    unsigned int* _countHostPtr;

    unsigned int* _countDevPtr;

    unsigned int _maxSize;

    unsigned int _itemSize;

public:
    CudaAtomicList()
    {
        _devPtr = nullptr;
        _hostPtr = nullptr;
        _countHostPtr = nullptr;
        _countDevPtr = nullptr;
        _maxSize = 0;
        _itemSize = 0;
    }

    ~CudaAtomicList()
    {
        cleanup();
    }

    cudaError_t init(unsigned int itemSize, unsigned int maxItems);

    unsigned int read(void* dest, unsigned int count);

    unsigned int size();

    void clear();

    void cleanup();
};

#endif
