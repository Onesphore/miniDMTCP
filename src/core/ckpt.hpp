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
    static void ckpt_context();
    static void ckpt_memory();
    // static void ckpt_fds();           // should be a plugin
  private:
    static int ckptImage_fd;
    int fds_num;
    
  public:
    static void take_checkpoint(int);
  public:
    static miniDMTCP theInstance;
  
};

miniDMTCP miniDMTCP::theInstance;
int miniDMTCP::ckptImage_fd = -1;
