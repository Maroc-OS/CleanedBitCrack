NAME=CLUnitTests
CPPSRC:=$(wildcard *.cpp)

all:
	cat ../clMath/secp256k1.cl secp256k1test.cl > cltest.cl
	${BINDIR}/embedcl cltest.cl cltest.cpp _secp256k1_test_cl

	${CXX} -o clunittest.bin cltest.cpp ${CPPSRC} ${INCLUDE} -I${OPENCL_INCLUDE} ${CXXFLAGS} ${LIBS} ${OPENCL_LIB} -lcommonutils -lclutil
	mkdir -p $(BINDIR)

	cp clunittest.bin $(BINDIR)/clunittest

clean:
	rm -rf *.a
	rm -rf *.o
	rm -rf *.gcno
	rm -rf *.gcda
	rm -rf cltest.cl
	rm -rf cltest.cpp
	rm -rf clunittest.bin
ifeq ($(PLATFORM),Darwin)
ifeq ($(BUILD_DEBUG),1)
	rm -rf clunittest.bin.dSYM
endif
endif
