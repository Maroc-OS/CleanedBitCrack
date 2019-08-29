NAME=CLKeySearchDevice
SRC:=$(wildcard *.cpp)
OBJS=$(SRC:.cpp=.o)

all:
	cat ../clMath/sha256.cl ../clMath/secp256k1.cl ../clMath/ripemd160.cl keysearch.cl > bitcrack.cl
	${BINDIR}/embedcl bitcrack.cl bitcrack_cl.cpp _bitcrack_cl

	for file in ${SRC} bitcrack_cl.cpp; do\
		${CXX} ${INCLUDE} -I${OPENCL_INCLUDE} ${CXXFLAGS} ${LIBS} -c $$file ${OPENCL_LIB} -llogger -lcommonutils -lclutil;\
	done
	mkdir -p ${LIBDIR}

ifeq ($(PLATFORM),Darwin)
	${CXX} ${LDFLAGS} -dynamiclib -undefined suppress -flat_namespace *.o -o ${LIBDIR}/lib$(NAME).dylib
else
	ar rvcs ${LIBDIR}/lib$(NAME).a *.o
	ranlib ${LIBDIR}/lib$(NAME).a
endif

clean:
	rm -rf *.a
	rm -rf *.o
	rm -rf *.gcno
	rm -rf *.gcda
	rm -rf bitcrack.cl
	rm -rf bitcrack_cl.cpp
