#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string.h>

#include <ucontext.h>
#include <setjmp.h>

#include <limits.h>

#define SIGNUM SIGUSR2 

#define PGSIZE sysconf(_SC_PAGESIZE)


class miniDMTCP {
  private:
    miniDMTCP();
    ~miniDMTCP();
  
  private:
    static int ckpt_fd;

  public:
    static void write_context();
    static void write_memMaps();
    // static void write_fds(); // should be a plugin
    static void take_checkpoint(int);
  
  public:
    static miniDMTCP theInstance;
  
};

miniDMTCP miniDMTCP::theInstance;
int miniDMTCP::ckpt_fd = -1;
