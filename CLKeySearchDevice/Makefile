NAME=CLKeySearchDevice
CPPSRC:=$(wildcard *.cpp)

all:
	cat ../clMath/sha256.cl ../clMath/secp256k1.cl ../clMath/ripemd160.cl keysearch.cl > bitcrack.cl
	${BINDIR}/embedcl bitcrack.cl bitcrack_cl.cpp _bitcrack_cl
	#xxd -i bitcrack.cl | sed "s!\([0-9a-f]\)$!\0, 0x00!" > bitcrack_cl.cpp
	#xxd -i bitcrack.cl > bitcrack_cl.cpp
	#sed -i -e 's/bitcrack_cl/_bitcrack_cl/g' bitcrack_cl.cpp

	for file in ${CPPSRC} bitcrack_cl.cpp; do\
		${CXX} -c $$file ${INCLUDE} -I${OPENCL_INCLUDE} ${CXXFLAGS};\
	done

ifeq ($(PLATFORM),Darwin)
	${CXX} ${LDFLAGS} -dynamiclib -undefined suppress -flat_namespace *.o -o ${LIBDIR}/lib$(NAME).dylib
else
	ar rvs ${LIBDIR}/lib$(NAME).a *.o
endif

clean:
	rm -rf *.a
	rm -rf *.o
	rm -rf *.gcno
	rm -rf *.gcda
	rm -rf bitcrack.cl
	rm -rf bitcrack_cl.cpp
