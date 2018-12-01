CXX=g++
CXXFLAGS=-g3 -O0 -fno-stack-protector
PLUGIN_RELATIVE_PATH=$(PWD)/src/plugins/
PLUGIN_ABS_PATH=$(abspath $(PLUGIN_RELATIVE_PATH))
LIB_RELATIVE_PATH=$(PWD)/bin
LIB_ABS_PATH=$(abspath $(LIB_RELATIVE_PATH))

default: libckpt.so myrestart

libckpt.so: ckpt.cpp ../utils/utils.cpp
	($(CXX) $(CXXFLAGS) -I$(PWD)/src/core -I$(PWD)/src -I$(PLUGIN_ABS_PATH)/fd/ \
		              -shared -fPIC -Wl,-rpath,$(LIB_ABS_PATH) -o $@ $^ \
			      -L$(LIB_ABS_PATH) -lfd)
	(mkdir -p $(LIB_ABS_PATH)) && \
	(mv $@ $(LIB_ABS_PATH))

myrestart: myrestart.cpp ../utils/utils.cpp
	(${CXX} ${CXXFLAGS} -static -I$(PWD)/src/core -I$(PWD)/src      \
        -Wl,-Ttext-segment=6400000 -Wl,-Tdata=6500000 -Wl,-Tbss=6600000 \
                                                    -o myrestart $^) && \
	(mkdir -p $(LIB_ABS_PATH)) && \
	(mv $@ $(LIB_ABS_PATH))
