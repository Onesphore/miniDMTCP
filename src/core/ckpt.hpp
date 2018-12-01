#ifndef CKPT_HPP
#define CKPT_HPP

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
#include <utils/utils.hpp>

#define SIGNUM SIGUSR2 

#define PGSIZE sysconf(_SC_PAGESIZE)


class miniDMTCP {
  private:
    miniDMTCP();
    ~miniDMTCP();
    static void pre_ckpting();
    static void ckpt_context();
    static void ckpt_memory();
    static void post_ckpting();
    // static void ckpt_fds();           // should be a plugin
  private:
    static int ckptImage_fd;
    static int fds_num;
    static ckptImg_header ckpt_h;
    
  public:
    static void take_checkpoint(int);
  public:
    static miniDMTCP theInstance;
  
};

miniDMTCP miniDMTCP::theInstance;
int miniDMTCP::ckptImage_fd = -1;
ckptImg_header miniDMTCP::ckpt_h;
int miniDMTCP::fds_num;

#endif // CKPT_HPP
