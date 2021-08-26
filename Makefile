
CUR_DIR=$(shell pwd)
PLATFORM=$(shell uname -s)

DIRS=Logger CommonUtils CmdParse CryptoUtil clUtil embedcl secp256k1lib AddressUtil KeyFinderLib CLKeySearchDevice CudaKeySearchDevice cudaMath cudaUtil

INCLUDE = $(foreach d, $(DIRS), -I$(CUR_DIR)/$d)

LIBDIR=$(CUR_DIR)/lib
BINDIR=$(CUR_DIR)/bin
LIBS+=-L$(LIBDIR)

# C++ options
ifeq ($(BUILD_DEBUG),1)
	CXXFLAGS=-DDEBUG -g -ggdb -O0
else
	CXXFLAGS=-DNDEBUG -O3 -flto -ffast-math
endif

LDFLAGS=
CXXFLAGS+=-std=c++17 -m64 -mssse3 -D_REETRANT -W -Wall -Wextra -pedantic -pthread

# Coverage variables
ifeq ($(BUILD_COVERAGE),1)
	CXXFLAGS+=--coverage -fprofile-arcs -ftest-coverage
	LDFLAGS+=--coverage
endif

ifeq ($(PLATFORM),Darwin)
    CXX=clang++
    CXXFLAGS+=-arch x86_64 -cl-mad-enable
else
ifeq ($(CXX),clang++)
    CXXFLAGS+=-arch x86_64 -cl-mad-enable
else
    CXX=g++
    CXXFLAGS+=-march=x86-64 -fgcse-sm
endif
endif

# CUDA variables
CUDA_HOME=/usr/local/cuda
CUDA_LIB=${CUDA_HOME}/lib64
CUDA_INCLUDE=${CUDA_HOME}/include
NVCC=${CUDA_HOME}/bin/nvcc
NVCCFLAGS=-std=c++17 -arch=sm_52 -gencode=arch=compute_52,code=sm_52 -gencode=arch=compute_60,code=sm_60 -gencode=arch=compute_61,code=sm_61 -gencode=arch=compute_70,code=sm_70 -gencode=arch=compute_75,code=sm_75 -gencode=arch=compute_80,code=sm_80 -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_86,code=compute_86 -Xptxas="-v" -Xcompiler "${CXXFLAGS}"
CUDA_MATH=$(CUR_DIR)/cudaMath

# OpenCL variables
OPENCL_VERSION=120
BUILD_OPENCL=1

ifeq ($(PLATFORM),Darwin)
	OPENCL_LIB=-framework OpenCL
	CXXFLAGS+=-Qunused-arguments
	OPENCL_INCLUDE=""
else
	OPENCL_LIB=-L${CUDA_LIB} -lOpenCL
	OPENCL_INCLUDE=${CUDA_INCLUDE}
endif

TARGETS=dir_logger dir_commonutils dir_cmdparse dir_secp256k1lib dir_cryptoutil dir_addressutil dir_keyfinderlib dir_keyfinder dir_addrgen

ifeq ($(BUILD_CUDA),1)
	TARGETS:=${TARGETS} dir_cudautil dir_cudaKeySearchDevice
endif

ifeq ($(BUILD_OPENCL),1)
	TARGETS:=${TARGETS} dir_embedcl dir_clutil dir_clKeySearchDevice dir_clunittest
	CXXFLAGS:=${CXXFLAGS}  -DCL_TARGET_OPENCL_VERSION=${OPENCL_VERSION} -DCL_HPP_MINIMUM_OPENCL_VERSION=${OPENCL_VERSION} -DCL_HPP_TARGET_OPENCL_VERSION=${OPENCL_VERSION}
endif

ifeq ($(BUILD_DEBUG),1)
	CXXFLAGS:=${CXXFLAGS} -Wconversion
endif

export PLATFORM
export INCLUDE
export LIBDIR
export BINDIR
export NVCC
export NVCCFLAGS
export LIBS
export CXX
export CXXFLAGS
export LDFLAGS
export CUDA_LIB
export CUDA_INCLUDE
export CUDA_MATH
export OPENCL_LIB
export OPENCL_INCLUDE
export BUILD_OPENCL
export BUILD_CUDA
export BUILD_DEBUG
export BUILD_COVERAGE

all:	${TARGETS}

dir_cudaKeySearchDevice: dir_logger dir_keyfinderlib dir_cudautil
	make --directory CudaKeySearchDevice

dir_clKeySearchDevice: dir_embedcl dir_logger dir_clutil dir_keyfinderlib
	make --directory CLKeySearchDevice

dir_embedcl:
	make --directory embedcl

dir_addressutil:	dir_commonutils dir_secp256k1lib dir_cryptoutil
	make --directory AddressUtil

dir_cmdparse:
	make --directory CmdParse

dir_cryptoutil:
	make --directory CryptoUtil

dir_keyfinderlib:	dir_commonutils dir_cmdparse dir_secp256k1lib dir_cryptoutil dir_addressutil dir_logger
	make --directory KeyFinderLib

KEYFINDER_DEPS=dir_keyfinderlib

ifeq ($(BUILD_CUDA), 1)
	KEYFINDER_DEPS:=$(KEYFINDER_DEPS) dir_cudaKeySearchDevice
endif

ifeq ($(BUILD_OPENCL),1)
	KEYFINDER_DEPS:=$(KEYFINDER_DEPS) dir_clKeySearchDevice
endif

dir_keyfinder:	$(KEYFINDER_DEPS)
	make --directory KeyFinder

dir_cudautil:
	make --directory cudaUtil

dir_clutil:	dir_commonutils
	make --directory clUtil

dir_secp256k1lib:	dir_cryptoutil
	make --directory secp256k1lib

dir_commonutils:
	make --directory CommonUtils

dir_cudainfo:
	make --directory cudaInfo

dir_logger:
	make --directory Logger

dir_addrgen:	dir_cmdparse dir_addressutil dir_secp256k1lib
	make --directory AddrGen

dir_clunittest:	dir_clutil
	make --directory CLUnitTests

clean:
	make --directory AddressUtil clean
	make --directory AddrGen clean
	make --directory CmdParse clean
	make --directory CryptoUtil clean
	make --directory KeyFinderLib clean
	make --directory KeyFinder clean
	make --directory cudaUtil clean
	make --directory secp256k1lib clean
	make --directory CommonUtils clean
	make --directory cudaInfo clean
	make --directory Logger clean
	make --directory clUtil clean
	make --directory CLKeySearchDevice clean
	make --directory CudaKeySearchDevice clean
	make --directory embedcl clean
	make --directory CLUnitTests clean
	rm -rf ${LIBDIR}
	rm -rf ${BINDIR}
