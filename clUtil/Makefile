NAME=clutil
SRC=$(wildcard *.cpp)
OBJS=$(SRC:.cpp=.o)

all:    ${SRC}
	for file in ${SRC} ; do\
		${CXX} -c $$file ${INCLUDE} -I${OPENCL_INCLUDE} ${CXXFLAGS} ${LIBS} ${OPENCL_LIB} -lutil;\
	done
	mkdir -p ${LIBDIR}

ifeq ($(PLATFORM),Darwin)
	${CXX} -dynamiclib -undefined suppress -flat_namespace ${OBJS} -o ${LIBDIR}/lib$(NAME).dylib
else
	ar rvs ${LIBDIR}/lib$(NAME).a ${OBJS}
endif

clean:
	rm -rf *.o