#include <myrestart.hpp>

void foo(void){return;}
inline void move_stack();

int main(int argc, char *argv[])
{
  static int ckpt_img_fd;
  static ckpt_header_t ckpt_h;

  assert(argc == 2);
  
  if ((ckpt_img_fd = open(argv[1], O_RDONLY)) == -1) {
    ERROR("open()");
  }
  if (read(ckpt_img_fd, &ckpt_h, sizeof(ckpt_header_t)) == -1) {
    ERROR("read()");
  }
  // Restart::move_stack();
  move_stack();
  Restart::restore_memoryMaps(ckpt_img_fd, ckpt_h);
  // Restart::restore_fds(ckpt_img_fd, ckpt_h);
  Restart::restore_context(ckpt_img_fd, ckpt_h);

  return 0;
}

inline void
move_stack() {
  // FIXME: should the following vars be declared 'static'?
  //        for now I don't see why they should
  static char *new_stack_addr = (char *)0x6700100; //
  static size_t new_stack_size = 0x6701000-0x6700000;
  if (mmap((void *)0x6700000, new_stack_size, PROT_READ|PROT_WRITE, 
      MAP_ANONYMOUS|MAP_PRIVATE, -1, 0) == (void *)-1) 
  {
    ERROR("mmap()");
  }
  asm volatile ("mov %0,%%rsp;" : : "g" (new_stack_addr) : "memory");
  Restart::foo();
  
  int maps_fd = open("/proc/self/maps", O_RDONLY);
  char *old_stack_addr;
  size_t old_stack_size;
  char line[128];
  while (_readline(maps_fd, line) != -1) {
    if (is_stack_line(line)) {
      // FIXME: there should be a utility function that 
      //        give a mapping line it returns both the 
      //        address of the mapping and its size.
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
  if (munmap(old_stack_addr, old_stack_size) == -1) {
    ERROR("munmap()");
  } 
 
}

void
Restart::restore_memoryMaps(int fd, ckpt_header_t ckpt_h) {
  // int   ckptImage_fd;
  off_t context_off;
  off_t memMaps_off;
  off_t memData_off;
  int   memMaps_num;
  
  int ckptImage_fd = fd;

  memMaps_off = ckpt_h.memMaps_offset;
  memMaps_num = ckpt_h.memMaps_num;
  memData_off   = ckpt_h.memData_offset;
  

  static memMap_t memMap;
  static int prot = PROT_READ | PROT_WRITE | PROT_EXEC; // FIXME: tooo generous!
  static int flags = MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS;
  static int i;
  for (i=1; i<=memMaps_num; ++i) {
    // read the ith memory section into memMap
    if (lseek(ckptImage_fd, memMaps_off, SEEK_SET) == (off_t)-1) {
      ERROR("lseek()");
    }
    memMaps_off += sizeof(memMap_t);

    if (read(ckptImage_fd, &memMap, sizeof(memMap_t)) == -1) {
      ERROR("read()");
    }     
    prot = prot | PROT_WRITE;
    if (memMap.permissions & (1<<2) == (1<<2))
      prot = prot | PROT_EXEC;    

    if (mmap(memMap.vaddr, memMap.data_size, prot, flags, 
                                      -1, 0) == (void *)-1)
    {
      ERROR("mmap()");
    }
    if (lseek(ckptImage_fd, memData_off, SEEK_SET) == (off_t)-1) {
      ERROR("lseek()");
    }
    memData_off += memMap.data_size;
    if (read(ckptImage_fd, memMap.vaddr, memMap.data_size) == -1) {
      ERROR("write()");
    }
       
    memset(&memMap, 0, sizeof(memMap));
  }
}


void Restart::restore_context(int fd, ckpt_header_t ckpt_h) {

}
// =====================================================================================
// 
// int main(int argc, char *argv[])
// {
//   //  move this process's stack.
//   static char *new_stack_addr = (char *)0x6700100; //
//   static size_t new_stack_size = 0x6701000-0x6700000;
//   if (mmap((void *)0x6700000, new_stack_size, PROT_READ|PROT_WRITE, 
//       MAP_ANONYMOUS|MAP_PRIVATE, -1, 0) == (void *)-1) 
//   {
//     ERROR("mmap()");
//   }
//  
//   static char myckpt[512];
//   strcpy(myckpt, argv[1]);
//   asm volatile ("mov %0,%%rsp;" : : "g" (new_stack_addr) : "memory");
//   foo();
//   
//   // unmap the original stack.
//   // read the address and size of the original stack.
//   int maps_fd = open("/proc/self/maps", O_RDONLY);
//   static char *old_stack_addr;
//   static size_t old_stack_size;
//   static char line[128];
//   while (_readline(maps_fd, line) != -1) {
//     if (is_stack_line(line)) {
//       char *line_p = line;
//       char *addr_begin, *addr_end;
//       char hex_str[17]; 
//       char *hex_str_p = hex_str;
//       memset(hex_str, 0, 17);
//       while (*line_p != '-') {
//         *hex_str_p++ = *line_p++;
//       }
//       
//       old_stack_addr = hexstring_to_int(hex_str);
//       
//       memset(hex_str, 0, 17);
//       hex_str_p = hex_str;
//       line_p++;// to get past "-"
//       while (*line_p != ' ') {
//         *hex_str_p++ = *line_p++;
//       }     
//     
//       addr_end = hexstring_to_int(hex_str);
//       old_stack_size = (size_t) (addr_end - old_stack_addr);
//       break;
//     }
//   }
//   if (close(maps_fd) == -1) {
//     ERROR("close()");
//   }
//   // unmap the old_stack region
//   static int ckptImage_fd;
//   ckptImage_fd = open(myckpt, O_RDONLY);
// 
//   if (munmap(old_stack_addr, old_stack_size) == -1) {
//     ERROR("munmap()");
//   } 
//   
//   static ucontext_t context;
//   /* ---map memory sections from the checkpoint image ---*/  
//   if (read(ckptImage_fd, &context, sizeof(context)) == -1) {
//     ERROR("read()");
//   }
//   
//   static int section_nbr;
//   // read the magic number, the number of sections
//   if (read(ckptImage_fd, &section_nbr, sizeof(int)) == -1) {
//     ERROR("read()");
//   }   
// 
//   // map the sections
//   static mem_section memMap;
//   static int prot = PROT_READ;
//   static int flags = MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS;
//   static int i;
//   for (i=1; i<=section_nbr; ++i) {
//     // read the ith memory section into memMap
//     if (read(ckptImage_fd, &memMap, sizeof(memMap)) == -1) {
//       ERROR("read()");
//     }
//          
//     // map the ith mem section into the virtual memory.
//     // for now give all region write permission so that
//     // they can be written to. 
//     prot = prot | PROT_WRITE;
//     if (memMap.executable == 'x')
//       prot = prot | PROT_EXEC;    
// 
//     if (mmap(memMap.address, memMap.size, prot, flags, 
//                                       -1, 0) == (void *)-1)
//     {
//       ERROR("mmap()");
//     }
//     if (read(ckptImage_fd, memMap.address, memMap.size) == -1) {
//       ERROR("write()");
//     }
//        
//     memset(&memMap, 0, sizeof(memMap));
//   }
// 
//   if (close(ckptImage_fd) == -1) {
//     ERROR("close()");
//   }
// 
//   if (setcontext(&context) == -1) {
//     ERROR("setcontext()");
//   }
// }
