CPPSRC:=$(wildcard *.cpp)

all:
ifeq ($(BUILD_CUDA), 1)
ifeq ($(PLATFORM),Darwin)
	${CXX} -DBUILD_CUDA -o cuKeyFinder.bin ${CPPSRC} ${INCLUDE} -I${CUDA_INCLUDE} ${CXXFLAGS} ${LIBS} ${OPENCL_LIB} -llogger -lcommonutils -lcmdparse -lsecp256k1 -lcryptoutil -lkeyfinder -laddressutil -lclutil -lclutil -lcudautil -lCudaKeySearchDevice -lcudadevrt -lcudart
else
	${CXX} -DBUILD_CUDA -o cuKeyFinder.bin ${CPPSRC} ${INCLUDE} -I${CUDA_INCLUDE} ${CXXFLAGS} ${LIBS} -Wl,--start-group ${OPENCL_LIB} -llogger -lcommonutils -lcmdparse -lsecp256k1 -lcryptoutil -lkeyfinder -laddressutil -lclutil -lclutil -lcudautil -lCudaKeySearchDevice -lcudadevrt -lcudart -Wl,--end-group
endif
	mkdir -p $(BINDIR)
	cp cuKeyFinder.bin $(BINDIR)/cuBitCrack
endif
ifeq ($(BUILD_OPENCL),1)
ifeq ($(PLATFORM),Darwin)
	${CXX} -DBUILD_OPENCL -o clKeyFinder.bin ${CPPSRC} ${INCLUDE} -I${OPENCL_INCLUDE} ${CXXFLAGS} ${LIBS} $(OPENCL_LIB) -llogger -lcommonutils -lcmdparse -lsecp256k1 -lcryptoutil -lkeyfinder -laddressutil -lclutil -lCLKeySearchDevice
else
	${CXX} -DBUILD_OPENCL -o clKeyFinder.bin ${CPPSRC} ${INCLUDE} -I${OPENCL_INCLUDE} ${CXXFLAGS} ${LIBS} -Wl,--start-group $(OPENCL_LIB) -llogger -lcommonutils -lcmdparse -lsecp256k1 -lcryptoutil -lkeyfinder -laddressutil -lclutil -lCLKeySearchDevice -Wl,--end-group
endif
	mkdir -p $(BINDIR)
	cp clKeyFinder.bin $(BINDIR)/clBitCrack
endif

clean:
	rm -rf *.gcno
	rm -rf *.gcda
	rm -rf cuKeyFinder.bin
	rm -rf clKeyFinder.bin
ifeq ($(PLATFORM),Darwin)
ifeq ($(BUILD_DEBUG),1)
	rm -rf cuKeyFinder.bin.dSYM
	rm -rf clKeyFinder.bin.dSYM
endif
endif
