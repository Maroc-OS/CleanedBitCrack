CPPSRC:=$(wildcard *.cpp)

all:
	${CXX} -o addrgen.bin ${CPPSRC} ${INCLUDE} -I${CUDA_INCLUDE} ${CXXFLAGS} ${LIBS} -laddressutil -lsecp256k1 -lcryptoutil -lsecp256k1 -lcommonutils -lcmdparse
	mkdir -p $(BINDIR)
	cp addrgen.bin $(BINDIR)/addrgen

clean:
	rm -rf *.gcno
	rm -rf *.gcda
	rm -rf addrgen.bin
ifeq ($(PLATFORM),Darwin)
ifeq ($(BUILD_DEBUG),1)
	rm -rf addrgen.bin.dSYM
endif
endif
