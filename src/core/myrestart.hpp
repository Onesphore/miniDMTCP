// FIXME: their should a class to store the offset,
//        in the ckpt image, of:
//        - context
//        - memory maps
//        - file descriptor metadata

// FIXME: These functions should in the above class
void read_context();
void read_memoryMaps();
void read_fdMetadata();

int main(int argc, char *argv[])
{
  //  move this process's stack.
  static char *new_stack_addr = (char *)0x6700100; //
  static size_t new_stack_size = 0x6701000-0x6700000;
  if (mmap((void *)0x6700000, new_stack_size, PROT_READ|PROT_WRITE, 
      MAP_ANONYMOUS|MAP_PRIVATE, -1, 0) == (void *)-1) 
  {
    ERROR("mmap()");
  }
 
  static char myckpt[512];
  strcpy(myckpt, argv[1]);
  asm volatile ("mov %0,%%rsp;" : : "g" (new_stack_addr) : "memory");
  foo();
  
  // unmap the original stack.
  // read the address and size of the original stack.
  int maps_fd = open("/proc/self/maps", O_RDONLY);
  static char *old_stack_addr;
  static size_t old_stack_size;
  static char line[128];
  while (_readline(maps_fd, line) != -1) {
    if (is_stack_line(line)) {
      char *line_p = line;
      char *addr_begin, *addr_end;
      char hex_str[17]; 
      char *hex_str_p = hex_str;
      memset(hex_str, 0, 17);
      while (*line_p != '-') {
        *hex_str_p++ = *line_p++;
      }
      
      old_stack_addr = hexstring_to_int(hex_str);
      
      memset(hex_str, 0, 17);
      hex_str_p = hex_str;
      line_p++;// to get past "-"
      while (*line_p != ' ') {
        *hex_str_p++ = *line_p++;
      }     
    
      addr_end = hexstring_to_int(hex_str);
      old_stack_size = (size_t) (addr_end - old_stack_addr);
      break;
    }
  }
  if (close(maps_fd) == -1) {
    ERROR("close()");
  }
  // unmap the old_stack region
  static int myckpt_fd;
  myckpt_fd = open(myckpt, O_RDONLY);

  if (munmap(old_stack_addr, old_stack_size) == -1) {
    ERROR("munmap()");
  } 
  
  static ucontext_t context;
  /* ---map memory sections from the checkpoint image ---*/  
  if (read(myckpt_fd, &context, sizeof(context)) == -1) {
    ERROR("read()");
  }
  
  static int section_nbr;
  // read the magic number, the number of sections
  if (read(myckpt_fd, &section_nbr, sizeof(int)) == -1) {
    ERROR("read()");
  }   

  // map the sections
  static mem_section msection;
  static int prot = PROT_READ;
  static int flags = MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS;
  static int i;
  for (i=1; i<=section_nbr; ++i) {
    // read the ith memory section into msection
    if (read(myckpt_fd, &msection, sizeof(msection)) == -1) {
      ERROR("read()");
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
      ERROR("mmap()");
    }
    if (read(myckpt_fd, msection.address, msection.size) == -1) {
      ERROR("write()");
    }
       
    memset(&msection, 0, sizeof(msection));
  }
  // read 'fds_num', the number of file descriptors
  int fds_num;
  if ((fds_num = read(myckpt_fd, &fds_num, sizeof(int))) == -1) {
    ERROR("read()");
  }
  // read 'fd metadata' 
  // FIXME: todo
  for (int i=0; i<fds_num; ++i) {

  }

  if (close(myckpt_fd) == -1) {
    ERROR("close()");
  }

  if (setcontext(&context) == -1) {
    ERROR("setcontext()");
  }
}
