#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define PGSIZE sysconf(_SC_PAGESIZE)
#define ERROR(msg)     \
do {                   \
  perror(msg);         \
  exit(EXIT_FAILURE);  \
} while(0);

// the structure to store useful info about 
// mapped memory sections.
typedef struct
{
  char readable;
  char writable;
  char executable;

  char *address;              // the address where memory starts.
  size_t size;                // the size of this memory section.
 
  bool is_stack;      // is this mem section a stack region
} mem_section;

int _readline(int, char*);
void fill_memsection(mem_section*, char*);
char* hexstring_to_int(char*);
int is_stack_line(char*);
int is_vvar_line(char*);
int is_vdso_line(char*);
int is_vsyscall_line(char*);
bool is_skip_region(char*);
void checkpoint(int);
