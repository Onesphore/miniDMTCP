CXX=g++
CXXFLAGS=-g3 -O0 -fno-stack-protector

CORE_RELATIVE_PATH=$(PWD)/src/core/
CORE_ABS_PATH=$(abspath $(CORE_RELATIVE_PATH))
# FIXME: The only plugin we have now is 'fd'.
#        When other plugins are added we should
#        change the following vars. Or use 'CMAKE'?
PLUGIN_RELATIVE_PATH=$(PWD)/src/plugins/fd
PLUGIN_ABS_PATH=$(abspath $(PLUGIN_RELATIVE_PATH))

MAKE_PATHS=$(CORE_ABS_PATH)
MAKE_PATHS:=$(MAKE_PATHS) $(PLUGIN_ABS_PATH)

all: $(MAKE_PATHS)
$(MAKE_PATHS):
	$(MAKE) -C $@

.PHONY: all $(MAKE_PATHS) clean
clean:
	rm -rf ./bin
