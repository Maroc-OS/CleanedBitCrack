NAME=keyfinder
CPPSRC:=$(wildcard *.cpp)


all:	cuda

cuda:
	for file in ${CPPSRC} ; do\
		${CXX} -c $$file ${INCLUDE} -I${CUDA_INCLUDE} ${CXXFLAGS};\
	done

ifeq ($(PLATFORM),Darwin)
	${CXX} ${LDFLAGS} -dynamiclib -undefined suppress -flat_namespace *.o -o ${LIBDIR}/lib$(NAME).dylib
else
	ar rvs ${LIBDIR}/lib$(NAME).a *.o
endif

clean:
	rm -f *.a
	rm -f *.o
	rm -rf *.cu.o
	rm -rf *.gcno
	rm -rf *.gcda
