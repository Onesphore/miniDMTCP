#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <ucontext.h>
#include <setjmp.h>

enum boolean {FALSE, TRUE};
typedef struct
{
  char readable;
  char writable;
  char executable;

  void *address;                 // the address where memory starts.
  size_t size;             // the size of this memory section.

  enum boolean is_stack;         // is this mem section a stack region
} mem_section;


void restore_memory(void){return;}

// synopses of other functions used.
int _readline(int, char*);
void* hexstring_to_int(char *);
int is_stack_line(char*);

int main(int argc, char *argv[])
{
  //  move this process's stack.
  static void *new_stack_addr = (void *)0x6700100; //
  static size_t new_stack_size = 0x6701000-0x6700000;
  if (mmap((void *)0x6700000, new_stack_size, PROT_READ|PROT_WRITE, 
      MAP_ANONYMOUS|MAP_PRIVATE, -1, 0) == (void *)-1)
  {
    perror("mmap()");
    exit(EXIT_FAILURE);
  }
 
  static char myckpt[512];
  strcpy(myckpt, argv[1]);
  asm volatile ("mov %0,%%rsp;" : : "g" (new_stack_addr) : "memory");
  restore_memory();
  
  // unmap the original stack.
  // read the address and size of the original stack.
  int maps_fd = open("/proc/self/maps", O_RDONLY);
  static void *old_stack_addr;
  static size_t old_stack_size;
  static char line[128];
  while (_readline(maps_fd, line) != -1)
  {
    if (is_stack_line(line))
    {
      char *line_p = line;
      void *addr_begin, *addr_end;
      char hex_str[17]; 
      char *hex_str_p = hex_str;
      memset(hex_str, 0, 17);
      while (*line_p != '-')
      {
        *hex_str_p++ = *line_p++;
      }
      
      old_stack_addr = hexstring_to_int(hex_str);
      
      memset(hex_str, 0, 17);
      hex_str_p = hex_str;
      line_p++;// to get past "-"
      while (*line_p != ' ')
      {
        *hex_str_p++ = *line_p++;
      }     
    
      addr_end = hexstring_to_int(hex_str);
      old_stack_size = (size_t) (addr_end - old_stack_addr);
      break;
    }
  }
  if (close(maps_fd) == -1){
    perror("close()");
    exit(EXIT_FAILURE);
  }
  // unmap the old_stack region
  static int myckpt_fd;
  myckpt_fd = open(myckpt, O_RDONLY);

  if (munmap(old_stack_addr, old_stack_size) == -1){
    perror("munmap()");
    exit(EXIT_FAILURE);
  } 
  
  static ucontext_t context;
  /* ---map memory sections from the checkpoint image ---*/  
  if (read(myckpt_fd, &context, sizeof(context)) == -1)
  {
    perror("read()");
    exit(EXIT_FAILURE);
  }
  // read is_ckpt pointer
  static int *is_ckpt_p;
  if (read(myckpt_fd, &(is_ckpt_p), sizeof(is_ckpt_p)) == -1){
    perror("read()");
    exit(EXIT_FAILURE);
  }
  
  static int section_nbr;
  // read the magic number, the number of sections
  if (read(myckpt_fd, &section_nbr, sizeof(int)) == -1)
  {
    perror("read()");
    exit(EXIT_FAILURE);
  }   

  // map the sections
  static mem_section msection;
  static int prot = PROT_READ;
  static int flags = MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS;
  static int i;
  for (i=1; i<=section_nbr; ++i)
  {
    // read the ith memory section into msection
    if (read(myckpt_fd, &msection, sizeof(msection)) == -1)
    {
      perror("read()");
      exit(EXIT_FAILURE);
    }
         
    // map the ith mem section into the virtual memory.
    // for now give all region write permission so that
    // they can be written to. 
    prot = prot | PROT_WRITE;
    if (msection.executable == 'x')
      prot = prot | PROT_EXEC;    

    if (mmap(msection.address, msection.size, prot, flags, 
                                      -1, 0) == (void *)-1)
    {
      perror("mmap()");
      exit(EXIT_FAILURE);
    }
   
    // write data in the mapped region
    if (read(myckpt_fd, msection.address, msection.size) == -1)
    {
      perror("write()");
      exit(EXIT_FAILURE);
    }
       
    memset(&msection, 0, sizeof(msection));
  }
  if (close(myckpt_fd) == -1){
    perror("close()");
    exit(EXIT_FAILURE);
  }
  // Then restart    
  *is_ckpt_p = 0;
  if (setcontext(&context) == -1){
    perror("setcontext()");
    exit(EXIT_FAILURE);
  }
}

void* hexstring_to_int(char *hexstring)
{
  void* ret_val=0;
  int base = 16;
  unsigned long long base_to_exp;

  size_t len = strlen(hexstring);
  int exp;
  for (exp=len-1; exp>=0; --exp)
  {
    if (exp == len-1){
      base_to_exp = 1;
    }
    else{
      base_to_exp *= 16;
    }

    if (hexstring[exp] >= 'a' && hexstring[exp] <= 'f'){
      ret_val += (hexstring[exp] - ('a'-10)) * base_to_exp;
    } else {
      ret_val += (hexstring[exp] -'0') * base_to_exp;
    }
  }
  return ret_val;
}

int _readline(int fd, char *line)
{
  char c;
  char *line_p = line;
  int cnt;
  while ((cnt = read(fd, &c, 1)) != 0)
  {
    *line_p++ = c;
    if (c == '\n')
      break;
  }

  if (cnt = 0){ // end of file read() return value = 0.
    return -1;
  }

  return 0;
}

int is_stack_line(char *line)
{
  char *line_p = line;
  while (*line_p != 's' && *line_p != '\n')
  {
    line_p++;
  }
  if (*line_p == '\n')
    return 0;

  if (*line_p == 's')
  {
    char stack_str[6];
    char *stack_str_p = stack_str;
    int i;
    for (i=0; i<5; ++i)
    {
      *stack_str_p++ = *line_p++;
    }
    *stack_str_p = 0;
    if (!strcmp(stack_str, "stack"))
      return 1;
  }

  return 0;
}
