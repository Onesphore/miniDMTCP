#include <ckpt.hpp>
#include <utils/utils.hpp>
// #include <regularFile/regularFD.hpp>

/*****************************************************************
 * When libminiDMTCP.so is loaded the static member 
 * variable 'theInstance', of type miniDMTCP, will be 
 * initialized.
 * At the time of initializing 'theInstance', miniDMTCP 
 * constructor will be called. That's when the 'signal handler'
 * will be installed.
 *****************************************************************/
miniDMTCP::miniDMTCP() {
  if (signal(SIGNUM, take_checkpoint) == SIG_ERR) {
    ERROR("signal()");
  }
}

// relinquish resources
miniDMTCP::~miniDMTCP() {
  if (close(ckpt_h.ckpt_img_fd) == -1) {
    ERROR("close()");
  }
}

void 
miniDMTCP::take_checkpoint(int signal) {
  off_t fds_num_offset = 0;
  off_t fds_metadata_offset = 0;

  if ((ckpt_h.ckpt_img_fd = open("myckpt.ckpt", O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1) {
    ERROR("open");
  }

  ckpt_h.context_offset = sizeof(ckpt_header_t)*2;
  ckpt_h.memMaps_offset = ckpt_h.context_offset + sizeof(ucontext_t);
  ckpt_h.fds_offset     = 2 * PGSIZE; // FIXME: check if it's big enough
  ckpt_h.memData_offset = 4 * PGSIZE; // FIXME: check if it's big enough
 
  ckpt_context();
  ckpt_memory();
 
  // FIXME: file descriptors here 

  if (lseek(ckpt_h.ckpt_img_fd, 0, SEEK_SET) == (off_t)-1) {
    ERROR("lseek()");
  }
  if (write(ckpt_h.ckpt_img_fd, &ckpt_h, sizeof(ckpt_header_t)) == -1) {
    ERROR("write()");
  }
}

void 
miniDMTCP::ckpt_context() {
  pid_t pid = getpid();
  memset(&context, 0, sizeof(ucontext_t));
  if (getcontext(&context) == -1) {
    ERROR("getcontext()");
  }

  if (pid == getpid()){ // to avoid taking a ckpt again on 'restart'
    if (lseek(ckpt_h.ckpt_img_fd, ckpt_h.context_offset, SEEK_SET) == (off_t)-1) {
      ERROR("lseek()");
    }
    if (write(ckpt_h.ckpt_img_fd, &context, sizeof(context)) == -1) {
      ERROR("write()");
    }
  }
}

void 
miniDMTCP::ckpt_memory() {
  int maps_fd;
  char line[128];
  memMap_t memMap;
  off_t memMaps_off;
  off_t memMaps_data_off;
  int memMaps_num = 0;
  char* access_limit = (char*) (long long)0xf000000000000000;

  if ((maps_fd = open("/proc/self/maps", O_RDONLY)) == -1) {
    ERROR("open()");
  }
 
  memMaps_off      = ckpt_h.memMaps_offset;
  memMaps_data_off = ckpt_h.memData_offset;
  while (_readline(maps_fd, line) != -1) {
    if (is_vvar_line(line) || is_vdso_line(line) || is_vsyscall_line(line) ||
        is_skip_region(line)) {
      continue;
    }
    fill_memMap(&memMap, line, memMaps_data_off);
    if (memMap.vaddr > access_limit) {
      continue;
    }
    
    // write mem. map metadata to ckpt image
    if (lseek(ckpt_h.ckpt_img_fd, memMaps_off, SEEK_SET) == (off_t)-1) {
      ERROR("lseek()");
    }
    if (write(ckpt_h.ckpt_img_fd, &memMap, sizeof(memMap_t)) == -1) {
      ERROR("write()");
    }

    // write mem. map data to ckpt image
    if (lseek(ckpt_h.ckpt_img_fd, memMaps_data_off, SEEK_SET) == (off_t)-1) {
      ERROR("lseek()");
    }
    {
      int nread = 0;
      int read_total=0;
      while (read_total != memMap.data_size){
        if ((nread = write(ckpt_h.ckpt_img_fd, memMap.vaddr+read_total, 
              memMap.data_size-read_total)) == -1) {
          ERROR("write()");
        }
        read_total += nread;
      }
    }

    memMaps_off += sizeof(memMap_t);
    memMaps_data_off += memMap.data_size;
    memMaps_num++;
  }
  ckpt_h.memMaps_num = memMaps_num;
  
  if (close(maps_fd) == -1) {
    ERROR("close()");
  }
}
