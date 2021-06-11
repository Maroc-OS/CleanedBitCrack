NAME=CLKeySearchDevice
LIBSRC=$(wildcard *.cpp)
LIBSRC:=$(LIBSRC) bitcrack_cl.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)
TARGETLIB=lib$(NAME).a

LOADLIBS = $(LIBS)
CXXFLAGS := $(INCLUDE) -I$(OPENCL_INCLUDE) $(CXXFLAGS) $(LOADLIBS) -Wl,--start-group $(OPENCL_LIB) -llogger -lcommonutils -lclutil -Wl,--end-group

$(TARGETLIB): $(LIBSRC)

TARGETS = $(TARGETLIB)

.PHONEY:
bitcrack_cl.cpp:
	rm -rf bitcrack.cl
	rm -rf bitcrack_cl.cpp
	cat ../clMath/sha256.cl ../clMath/secp256k1.cl ../clMath/ripemd160.cl bloomfilter.cl bitcoin.cl keysearch.cl > bitcrack.cl
	$(BINDIR)/embedcl bitcrack.cl bitcrack_cl.cpp _bitcrack_cl

all:	$(LIBSRC)
	for file in $(LIBSRC); do\
		$(CXX) $(CXXFLAGS) -c $$file;\
	done

$(TARGETS): $(LIBOBJ)
	mkdir -p $(LIBDIR)
ifeq ($(PLATFORM),Darwin)
	$(CXX) $(LDFLAGS) -dynamiclib -undefined suppress -flat_namespace $(LIBOBJ) -o $(LIBDIR)/lib$(NAME).dylib
else
	ar rcvs $(LIBDIR)/$(TARGETLIB) $(LIBOBJ)
	ranlib $(LIBDIR)/$(TARGETLIB)
endif

clean:
	rm -rf *.a
	rm -rf *.o
	rm -rf *.gcno
	rm -rf *.gcda
	rm -rf bitcrack.cl
	rm -rf bitcrack_cl.cpp
