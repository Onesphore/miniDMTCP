CC=gcc
CCFLAGS=-g3 -O0 -fno-stack-protector
PATH_=$(PWD)/../../bin
LIBPATH=$(realpath $(PATH_))

hello: hello.c
	$(CC) $(CCFLAGS) -o $@ $^

restart:
	$(LIBPATH)/myrestart ./myckpt.ckpt

check: hello
	(sleep 3 && kill -12 `pgrep -n hello` && \
	 sleep 2 && pkill -9 -n hello && make restart) & \
	 # sleep 12 && pkill -9 -n myrestart & 
	 LD_PRELOAD=$(LIBPATH)/libckpt.so ./hello

.PHONY: clean
clean:
	rm -rf hello ./myckpt.ckpt
