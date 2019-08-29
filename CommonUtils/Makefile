NAME=commonutils
SRC=$(wildcard *.cpp)
OBJS=$(SRC:.cpp=.o)

all:    ${SRC}
	for file in ${SRC} ; do\
		${CXX} ${INCLUDE} ${CXXFLAGS} -c $$file;\
	done
	mkdir -p ${LIBDIR}

ifeq ($(PLATFORM),Darwin)
	${CXX} ${LDFLAGS} -dynamiclib -undefined suppress -flat_namespace ${OBJS} -o ${LIBDIR}/lib$(NAME).dylib
else
	ar rvcs ${LIBDIR}/lib$(NAME).a ${OBJS}
	ranlib ${LIBDIR}/lib$(NAME).a
endif

clean:
	rm -rf *.a
	rm -rf *.o
	rm -rf *.gcno
	rm -rf *.gcda
