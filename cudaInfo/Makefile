CPPSRC:=$(wildcard *.cpp)

all:
	${CXX} -o cudainfo.bin ${CPPSRC} ${INCLUDE} -I${CUDA_INCLUDE} ${CXXFLAGS} ${LIBS} -L${CUDA_LIB} -lcudautil -lcudart 
	mkdir -p $(BINDIR)
	cp cudainfo.bin $(BINDIR)/cudainfo

clean:
	rm -rf *.gcno
	rm -rf *.gcda
	rm -rf cudainfo.bin
ifeq ($(PLATFORM),Darwin)
ifeq ($(BUILD_DEBUG),1)
	rm -rf cudainfo.bin.dSYM
endif
endif
