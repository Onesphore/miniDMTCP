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

ucontext_t context;
void checkpoint(int);

/* 
-is_ckpt. This variable is used to check if it's checkpoint time
or restart time.

-is_ckpt_p. The pointer to "is_ckpt" is stored in the ckpt image
and before calling "setcontext()" in myrestart this pointer is
used to change the value of "is_ckpt".
This is possible since the memory of "hello" was copied
so was is_ckpt.
*/
int is_ckpt = 31; // just a random number
int* is_ckpt_p = &is_ckpt;

// this function will be run before the main function
// of hello.
// It will install a signal handler. Once the signal is
// received then "hello" will be checkpointed.
__attribute__((constructor)) void before_main(void)
{
  if (signal(SIGNUM, checkpoint) == SIG_ERR)
  {
    perror("signal()");
    exit(EXIT_FAILURE);
  }
}
//
enum boolean {FALSE, TRUE};
// the structure to store useful info about 
// mapped memory sections.
typedef struct
{
  char readable;
  char writable;
  char executable;

  void *address;              // the address where memory starts.
  size_t size;                // the size of this memory section.
 
  enum boolean is_stack;      // is this mem section a stack region
} mem_section;

// synopses of functions used in "checkpoint()"
void ckpt_memMaps();
int _readline(int, char*);
void fill_memsection(mem_section*, char*);
void* hexstring_to_int(char*);
int is_stack_line(char*);
int is_vvar_line(char*);
int is_vdso_line(char*);
int is_vsyscall_line(char*);


// the signal handler.
void 
checkpoint(int signal_USR2)
{
  pid_t pid = getpid();
  memset(&context, 0, sizeof(context));
  if (getcontext(&context) == -1){
    perror("getcontext()");
    exit(EXIT_FAILURE);
  }

  if (pid == getpid()){
    int myckpt_fd;
    if ((myckpt_fd = open("myckpt", O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1)
    {
      perror("open()");
      exit(EXIT_FAILURE);
    }
    // save the context.
    if (write(myckpt_fd, &context, sizeof(context)) == -1){
      perror("write()");
      exit(EXIT_FAILURE);
    }
    // save "is_ckpt" address to myckpt.
    if (write(myckpt_fd, &(is_ckpt_p), sizeof(int*)) == -1){
      perror("write()");
      exit(EXIT_FAILURE);
    }
    
    // open the current process("hello") mapping image
    int maps_fd;
    if ((maps_fd = open("/proc/self/maps", O_RDONLY)) == -1)
    {
      perror("open()");
      exit(EXIT_FAILURE);
    }
  
    // leave space for the magic number.
    if (lseek(myckpt_fd, sizeof(int), SEEK_CUR) == (off_t)-1)
    {
      perror("lseek()");
      exit(EXIT_FAILURE);
    }

    // read the current process("hello") mapped memory sections
    // and write them to the ckpt image.
    char line[128];
    mem_section msection;
    int section_nbr = 0;
    void *access_limit = (void *)(long long)0xf000000000000000;

    while (_readline(maps_fd, line) != -1)
    {
      // check if it's any of the region we don't need to write
      // to the ckpt image file.
      if (is_vvar_line(line)){
        continue;
      }
      if (is_vdso_line(line)){
        continue;
      }
      if (is_vsyscall_line(line)){
        continue;
      }      

      fill_memsection(&msection, line);
      if (msection.address > access_limit){
        continue;
      }
      section_nbr++;
      if (write(myckpt_fd, &msection, sizeof(msection)) == -1){
        perror("write()");
        exit(EXIT_FAILURE);
      }   
      
      int nread = 0;
      int read_total=0;
      while (read_total != msection.size){
        if ((nread = write(myckpt_fd, msection.address+read_total, 
              msection.size-read_total)) == -1){
          perror("write()");
          exit(EXIT_FAILURE);
        }
        read_total += nread;
      }
    }

    // move the offset of "myckpt_fd" to where "section_nbr"
    // will be written. 
    if (lseek(myckpt_fd, sizeof(context)+sizeof(int*), SEEK_SET) == 
        (off_t)-1)
    {
      perror("lseek()");
      exit(EXIT_FAILURE);
    } 
    // section_nbr: the number of memory sections written to 
    // the ckpt image file. Save this number also.
    if (write(myckpt_fd, &section_nbr, sizeof(section_nbr)) == -1)
    {
      perror("write()");
      exit(EXIT_FAILURE);
    }

    if (close(myckpt_fd) == -1){
       perror("close()");
       exit(EXIT_FAILURE);
     }
     if (close(maps_fd) == -1){
       perror("close()");
       exit(EXIT_FAILURE);
     }
  //
  }
}


int 
_readline(int fd, char *line)
{
  memset(line, 0, 128);

  char c;
  char *line_p = line;
  int cnt;
  while ((cnt = read(fd, &c, 1)) !=0)
  {
    *line_p++ = c;
    if (c == '\n'){
      break;
    }
  }
  
  if (cnt == 0){ // end of file read() return value = 0.
    return -1;
  }

  return 0;
}


void 
fill_memsection(mem_section *msection_p, char *line)
{
  memset(msection_p, 0, sizeof(mem_section));
  char *line_p = line;
  //1. get the address where this memory section begins
  void *addr_begin, *addr_end;
  char hex_str[17];
  char *hex_str_p = hex_str;

  memset(hex_str, 0, 17);
  while (*line_p != '-')
  {
    *hex_str_p++ = *line_p++;
  }

  addr_begin = hexstring_to_int(hex_str);

  //  . get the address where this memory section ends
  hex_str_p = hex_str;
  memset(hex_str, 0, 17);
  line_p++; // to get past "-"
  while (*line_p != ' ')
  {
    *hex_str_p++ = *line_p++;
  }
  addr_end = hexstring_to_int(hex_str);
 
  msection_p->address = addr_begin;
  msection_p->size = (size_t) (addr_end - addr_begin);
 
  line_p++; // get past " "

  //3. get access mode: r/w/x
  msection_p->readable = *line_p++;
  msection_p->writable = *line_p++;
  msection_p->executable = *line_p++;

  //4. check if this memory section is a stack region
  while (*line_p != 's' && *line_p != '\n')
  {
    line_p++;
  }
  if (*line_p == '\n')
  {
    msection_p->is_stack = FALSE;
    return;
  }  


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
      msection_p->is_stack = TRUE;
    
    return;
  }

  msection_p->is_stack = FALSE;
  return;
}

void ckpt_memMaps() {
 ;
}

void* 
hexstring_to_int(char *hexstring)
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

int 
is_stack_line(char *line)
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

int 
is_vvar_line(char *line)
{
  char *line_p = line;
  while (*line_p != 'v' && *line_p != '\n')
  {
    line_p++;
  }
  if (*line_p == '\n')
    return 0;

  if (*line_p == 'v')
  {
    char vvar_str[5];
    char *vvar_str_p = vvar_str;
    int i;
    for (i=0; i<4; ++i)
    {
      *vvar_str_p++ = *line_p++;
    }
    *vvar_str_p = 0;
    if (!strcmp(vvar_str, "vvar"))
      return 1;
  }

  return 0;
}

int 
is_vdso_line(char *line)
{
  char *line_p = line;
  while (*line_p != 'v' && *line_p != '\n')
  {
    line_p++;
  }
  if (*line_p == '\n')
    return 0;

  if (*line_p == 'v')
  {
    char vdso_str[5];
    char *vdso_str_p = vdso_str;
    int i;
    for (i=0; i<4; ++i)
    {
      *vdso_str_p++ = *line_p++;
    }
    *vdso_str_p = 0;
    if (!strcmp(vdso_str, "vdso"))
      return 1;
  }

  return 0;
}

int 
is_vsyscall_line(char *line)
{
  char *line_p = line;
  while (*line_p != 'v' && *line_p != '\n')
  {
    line_p++;
  }
  if (*line_p == '\n')
    return 0;

  if (*line_p == 'v')
  {
    char vsyscall_str[9];
    char *vsyscall_str_p = vsyscall_str;
    int i;
    for (i=0; i<8; ++i)
    {
      *vsyscall_str_p++ = *line_p++;
    }
    *vsyscall_str_p = 0;
    if (!strcmp(vsyscall_str, "vsyscall"))
      return 1;
  }

  return 0;
}

