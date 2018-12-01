#include <myrestart.hpp>

void foo(void){return;}

int main(int argc, char *argv[])
{
  // './myrestart myckpt.ckpt' should be the command
  assert(argc == 2);
  strcpy(Restart::theInstance.ckptImage_fileName, argv[1]);
  Restart::theInstance.move_stack();
  Restart::theInstance.restore_memoryMaps();
  Restart::theInstance.restore_fds();
  Restart::theInstance.restore_context();

  return 0;
}

void
Restart::move_stack() {
  // FIXME: should the following vars be declared 'static'?
  //        for now I don't see why they should
  char *new_stack_addr = (char *)0x6700100; //
  size_t new_stack_size = 0x6701000-0x6700000;
  if (mmap((void *)0x6700000, new_stack_size, PROT_READ|PROT_WRITE, 
      MAP_ANONYMOUS|MAP_PRIVATE, -1, 0) == (void *)-1) 
  {
    ERROR("mmap()");
  }
  asm volatile ("mov %0,%%rsp;" : : "g" (new_stack_addr) : "memory");
  Restart::theInstance.foo();
  
  // FIXME: too much 'static'
  //        we only changed 'sp' register's content, not 'fp'.
  //        It should not matter that some variable are still in 
  //        old stack.
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

// FIXME: first change the format of the ckpt image
void
Restart::restore_memoryMaps() {
  int   ckptImage_fd;
  off_t context_off;
  off_t memMapsHeader_off;
  off_t memMapsData_off;
  int   memMaps_num;

  if ((ckptImage_fd = open(Restart::theInstance.ckptImage_fileName, 
				                   O_RDONLY)) == -1) {
    ERROR("open()");
  }

  // read the ckpt image header
  ckptImg_header ckpt_h;
  if (read(ckptImage_fd, &ckpt_h, sizeof(ckptImg_header)) == -1) {
    ERROR("read()");
  }
  // context_off       = ckpt_h.context_off;
  memMapsHeader_off = ckpt_h.memMapsHeader_off;
  memMaps_num       = ckpt_h.memMapsNum;
  memMapsData_off   = ckpt_h.memMapsData_off;
  

  static memMap_t memMap;
  static int prot = PROT_READ | PROT_WRITE | PROT_EXEC; // FIXME: tooo generous!
  static int flags = MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS;
  static int i;
  for (i=1; i<=memMaps_num; ++i) {
    // read the ith memory section into memMap
    if (lseek(ckptImage_fd, memMapsHeader_off, SEEK_SET) == (off_t)-1) {
      ERROR("lseek()");
    }
    memMapsHeader_off += sizeof(memMap_t);
    if (read(ckptImage_fd, &memMap, sizeof(memMap_t)) == -1) {
      ERROR("read()");
    }
         
    // map the ith mem section into the virtual memory.
    // for now give all region write permission so that
    // they can be written to. 
    prot = prot | PROT_WRITE;
    if (memMap.executable == true)
      prot = prot | PROT_EXEC;    

    if (mmap(memMap.v_addr, memMap.size, prot, flags, 
                                      -1, 0) == (void *)-1)
    {
      ERROR("mmap()");
    }
    if (lseek(ckptImage_fd, memMapsData_off, SEEK_SET) == (off_t)-1) {
      ERROR("lseek()");
    }
    memMapsData_off += memMap.size;
    if (read(ckptImage_fd, memMap.v_addr, memMap.size) == -1) {
      ERROR("write()");
    }
       
    memset(&memMap, 0, sizeof(memMap));
  }
}

void Restart::restore_fds() {

}

void Restart::restore_context() {

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
