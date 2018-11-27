DIR := ${CURDIR}
INCLUDE=$(PWD)/include
CXX=g++
CXXFLAGS=-g3 -O0 -fno-stack-protector
LIBPATH=../bin

hello: hello.cpp
	${CXX} ${CXXFLAGS} -o $@ $^ -L$(LIBPATH) -lckpt

restart:
	$(LIBPATH)/myrestart ./myckpt.ckpt

gdb: libckpt.so hello
	gdb ./hello

gdb-restart: ../bin/myrestart myckpt.ckpt
	gdb --args $(LIBPATH)/myrestart ./myckpt.ckpt

check: hello
	(sleep 3 && kill -12 `pgrep -n hello` && \
	sleep 2 && pkill -9 -n hello && make restart) & 
	LD_PRELOAD=$(LIBPATH)/libckpt.so ./hello

clean:
	rm -rf hello myckpt.ckpt
