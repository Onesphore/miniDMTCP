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
#include <common/ckptImage.hpp>
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

  private:
    static ckpt_header_t ckpt_h;
    static ucontext_t context;   
  public:
    static void take_checkpoint(int);
  public:
    static miniDMTCP theInstance;
  
};

miniDMTCP miniDMTCP::theInstance;
ckpt_header_t miniDMTCP::ckpt_h;
ucontext_t miniDMTCP::context;

#endif // CKPT_HPP
