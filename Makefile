DIR := ${CURDIR}
CXX=g++
CXXFLAGS=-g3 -O0 -fno-stack-protector

all:
	cd src/core && $(MAKE)


.PHONY: clean
clean:
	rm -rf ./bin
