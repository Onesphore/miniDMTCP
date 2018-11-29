DIR := ${CURDIR}
CXX=g++
CXXFLAGS=-g3 -O0 -fno-stack-protector

default: libckpt.so myrestart

libckpt.so: core/ckpt.cpp utils/utils.cpp
	($(CXX) $(CXXFLAGS) -I$(PWD) -shared -fPIC \
	                -Wl,-rpath,$(DIR)/bin -o $@ $^) && \
	(mkdir -p bin) && \
	(mv $@ ./bin)
	

myrestart: src/myrestart.cpp
	(${CXX} ${CXXFLAGS} -static \
        -Wl,-Ttext-segment=6400000 -Wl,-Tdata=6500000 -Wl,-Tbss=6600000 \
                                                        -o myrestart $<) && \
	(mkdir -p bin) && \
	(mv $@ ./bin)

.PHONY: clean
clean:
	rm -rf ./bin


