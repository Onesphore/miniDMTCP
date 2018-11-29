DIR := ${CURDIR}
INCLUDE=$(PWD)/include
CXX=g++
CXXFLAGS=-g3 -O0 -fno-stack-protector

libfiledescriptors.so: src/*.cpp
	($(CXX) $(CXXFLAGS) -I$(INCLUDE) -shared -fPIC \
	                -Wl,-rpath,$(DIR)/bin -o $@ $^) && \
	(mkdir -p bin) && \
	(mv $@ ./bin)

.PHONY: clean
clean:
	rm -rf ./bin


