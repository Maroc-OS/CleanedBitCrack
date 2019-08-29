NAME=CLUnitTests
CPPSRC=$(wildcard *.cpp)
CPPSRC:=$(wildcard *.cpp) cltest.cpp
CPPBIN=clunittest
TARGETBIN=$(NAME).bin

LOADLIBS = $(LIBS)
ifeq ($(PLATFORM),Darwin)
CXXFLAGS := $(INCLUDE) -I$(OPENCL_INCLUDE) $(CXXFLAGS) $(LOADLIBS) $(OPENCL_LIB) -llogger -lcommonutils -lclutil
else
CXXFLAGS := $(INCLUDE) -I$(OPENCL_INCLUDE) $(CXXFLAGS) $(LOADLIBS) -Wl,--start-group $(OPENCL_LIB) -llogger -lcommonutils -lclutil -Wl,--end-group
endif

all:	$(TARGETBIN)

$(TARGETBIN): $(CPPSRC)
	$(CXX) -o $(TARGETBIN) $(CPPSRC) $(CXXFLAGS)
	mkdir -p $(BINDIR)
	cp $(TARGETBIN) $(BINDIR)/$(CPPBIN)

.PHONEY:
cltest.cpp:
	rm -rf cltest.cl
	rm -rf cltest.cpp
	cat ../clMath/secp256k1.cl secp256k1test.cl > cltest.cl
	$(BINDIR)/embedcl cltest.cl cltest.cpp _secp256k1_test_cl

clean:
	rm -rf *.a
	rm -rf *.o
	rm -rf *.gcno
	rm -rf *.gcda
	rm -rf cltest.cl
	rm -rf cltest.cpp
	rm -rf $(TARGETBIN)
ifeq ($(PLATFORM),Darwin)
ifeq ($(BUILD_DEBUG),1)
	rm -rf $(TARGETBIN).dSYM
endif
endif
