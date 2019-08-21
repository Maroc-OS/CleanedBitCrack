NAME=secp256k1
SRC=$(wildcard *.cpp)
OBJS=$(SRC:.cpp=.o)

all:    ${SRC}
	for file in ${SRC} ; do\
		${CXX} -c $$file ${INCLUDE} ${CXXFLAGS} ${LIBS} -lcryptoutil;\
	done
	mkdir -p ${LIBDIR}

ifeq ($(PLATFORM),Darwin)
	${CXX} ${LDFLAGS} -dynamiclib -undefined suppress -flat_namespace ${OBJS} -o ${LIBDIR}/lib$(NAME).dylib
else
	ar rvs ${LIBDIR}/lib$(NAME).a ${OBJS} 
endif

clean:
	rm -rf *.a
	rm -rf *.o
	rm -rf *.gcno
	rm -rf *.gcda
