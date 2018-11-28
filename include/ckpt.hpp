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
    void ckpt_context();
    void ckpt_memory();
    void ckpt_fds();           // should be a plugin
  private:
    int ckpt_fd = -1;
    
  public:
    void take_checkpoint(int);
  public:
    static miniDMTCP theInstance;
  
};

miniDMTCP miniDMTCP::theInstance;
