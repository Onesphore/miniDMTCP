DIR := ${CURDIR}
INCLUDE=$(PWD)/include
CXX=g++
CXXFLAGS=-g3 -O0 -fno-stack-protector

libckpt.so: src/ckpt.cpp src/utils.cpp
	($(CXX) $(CXXFLAGS) -I$(INCLUDE) -shared -fPIC \
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


