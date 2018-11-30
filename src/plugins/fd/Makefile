SRC_RELATIVE_PATH=$(PWD)/src/plugins/fd
SRC_ABS_PATH=$(abspath $(SRC_RELATIVE_PATH))
BIN_RELATIVE_PATH=$(PWD)/bin
BIN_ABS_PATH=$(abspath $(BIN_RELATIVE_PATH))

CXX=g++
CXXFLAGS=-g3 -O0 -fno-stack-protector

# libfd.so: $(wildcard $(SRC_ABS_PATH)/regularFile/*.cpp)
libfd.so: $(wildcard $(SRC_ABS_PATH)/pty/*.cpp) \
	  $(wildcard $(SRC_ABS_PATH)/regularFile/*.cpp) \
	  $(wildcard $(SRC_ABS_PATH)/socket/*.cpp)
	  ($(CXX) $(CXXFLAGS) -I$(SRC_ABS_PATH) -shared -fPIC \
	                 -Wl,-rpath,$(BIN_ABS_PATH)/bin -o $@ $^) && \
	  (mkdir -p $(BIN_ABS_PATH)) && \
	  (mv $@ $(BIN_ABS_PATH))
