SRC_RELATIVE_PATH=$(PWD)/../
SRC_ABS_PATH=$(abspath $(SRC_RELATIVE_PATH))
BIN_RELATIVE_PATH=$(PWD)/../../../../
BIN_ABS_PATH=$(abspath $(BIN_RELATIVE_PATH))

CXX=g++
CXXFLAGS=-g3 -O0 -fno-stack-protector

# libfd.so: $(SRC_ABS_PATH)/pty/*.cpp $(SRC_ABS_PATH)/regularFile/*.cpp $(SRC_ABS_PATH)/socket/*.cpp
libfd.so: $(SRC_ABS_PATH)/regularFile/regularFD.cpp
	($(CXX) $(CXXFLAGS) -I$(INCLUDE) -shared -fPIC \
	                -Wl,-rpath,$(DIR)/bin -o $@ $^) && \
	(mkdir -p $(BIN_ABS_PATH)) && \
	(mv $@ $(BIN_ABS_PATH))
