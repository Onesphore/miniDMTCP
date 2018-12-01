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

#define PGSIZE sysconf(_SC_PAGESIZE)
#define ERROR(msg)     \
do {                   \
  perror(msg);         \
  exit(EXIT_FAILURE);  \
} while(0);

struct memMap_t
{
  bool   readable;
  bool   writable;
  bool   executable;
  char*  v_addr;                // the address where memory starts.
  off_t  file_addr;             // offset of data in the ckpt image
  size_t size;                  // the size of this memory section.
  bool   is_stack;              // is this mem section a stack region
};

/** ANATOMY OF THE CKPT IMAGE **/
struct ckptImg_header {
  off_t     context_off;
  off_t     memMapsHeader_off;
  uint32_t  memMapsNum;
  off_t     fdsMetadata_off;
  uint32_t  fdsMetadataNum;
  off_t     memMapsData_off;
};

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
