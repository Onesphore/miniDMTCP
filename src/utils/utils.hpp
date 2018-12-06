#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ucontext.h>
#include <setjmp.h>
#include <stdint.h>

#include <common/ckptImage.hpp>

#define PGSIZE sysconf(_SC_PAGESIZE)
#define ERROR(msg)     \
do {                   \
  perror(msg);         \
  exit(EXIT_FAILURE);  \
} while(0);

// useful functions for reading memory maps
int _readline(int, char*);
void fill_memMap(memMap_t*, char*, off_t);
char* hexstring_to_int(char*);
int is_stack_line(char*);
int is_vvar_line(char*);
int is_vdso_line(char*);
int is_vsyscall_line(char*);
bool is_skip_region(char*);

#endif// UTILS_HPP
