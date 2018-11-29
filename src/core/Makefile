CXX=g++
CXXFLAGS=-g3 -O0 -fno-stack-protector
PATH_=$(PWD)/../../bin
LIBPATH=$(abspath $(PATH_))

default: libckpt.so myrestart

libckpt.so: ckpt.cpp ../utils/utils.cpp
	($(CXX) $(CXXFLAGS) -I$(PWD)/.. -I$(PWD) -shared -fPIC \
	                -Wl,-rpath,$(LIBPATH) -o $@ $^) && \
	(mkdir -p $(LIBPATH)) && \
	(mv $@ $(LIBPATH))

myrestart: myrestart.cpp ../utils/utils.cpp
	(${CXX} ${CXXFLAGS} -static -I$(PWD)/.. -I$(PWD) \
        -Wl,-Ttext-segment=6400000 -Wl,-Tdata=6500000 -Wl,-Tbss=6600000 \
                                                    -o myrestart $^) && \
	(mkdir -p $(LIBPATH)) && \
	(mv $@ $(LIBPATH))
