NAME=embedcl
CPPSRC:=$(wildcard *.cpp)

all:
	${CXX} -o ${NAME} ${CPPSRC} ${INCLUDE} ${CXXFLAGS}
	mkdir -p $(BINDIR)
	cp ${NAME} $(BINDIR)/${NAME}

clean:
	rm -rf ${NAME}
ifeq ($(PLATFORM),Darwin)
ifeq ($(BUILD_DEBUG),1)
	rm -rf ${NAME}.dSYM
endif
endif