#include <ckpt.hpp>
#include <utils/utils.hpp>
#include <regularFile/regularFD.hpp>

ucontext_t context;

// FIXME: member variables initialization please!

// miniDMTCP class member functions definition
miniDMTCP::miniDMTCP() {
  if (signal(SIGNUM, take_checkpoint) == SIG_ERR) {
    ERROR("signal()");
  }
}

// relinquish resources
miniDMTCP::~miniDMTCP() {
  if (close(ckptImage_fd) == -1) {
    ERROR("close()");
  }
}

void 
miniDMTCP::take_checkpoint(int signal) {
  off_t fds_num_offset = 0;
  off_t fds_metadata_offset = 0;

  if (( ckptImage_fd = open("myckpt.ckpt", O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1) {
    ERROR("open");
  }
  ckpt_h.context_off = sizeof(ckptImg_header)*2;
  ckpt_h.memMapsHeader_off = ckpt_h.context_off + sizeof(ucontext_t);
  ckpt_h.fdsMetadata_off = 2 * PGSIZE; // FIXME: check if it's big enough
  ckpt_h.memMapsData_off = 4 * PGSIZE; // FIXME: check if it's big enough
 
  ckpt_context();
  ckpt_memory();

  off_t fd_metadata_off = ckpt_h.fdsMetadata_off;  
  // file descriptors
  fd_t* fds_ptr;
  fileDescriptor* fileDescr_ptr;
  int fds_num;
  fileDescriptor::getFileDescriptors(&fds_ptr, &fds_num);
  for (int i=0; i<fds_num; ++i) {
    switch (fds_ptr[i].fd_type) {
      case REGULAR:
	fileDescr_ptr = new regularFD();
        break;

      case SOCKET:
	fileDescr_ptr = new socketFD();
	break;

      case PTY:
	fileDescr_ptr = new ptyFD();
	break;
    }

    fileDescr_ptr->writeFdToCkptImg(ckptImage_fd, fd_metadata_off, fds_ptr[i]);
    fd_metadata_off += sizeof(fd_t);
  }

  ckpt_h.fdsMetadataNum = fds_num;
  if (lseek(ckptImage_fd, 0, SEEK_SET) == (off_t)-1) {
    ERROR("lseek()");
  }
  if (write(ckptImage_fd, &ckpt_h, sizeof(ckptImg_header)) == -1) {
    ERROR("write()");
  }
}

void 
miniDMTCP::ckpt_context() {
  pid_t pid = getpid();
  memset(&context, 0, sizeof(context));
  if (getcontext(&context) == -1) {
    ERROR("getcontext()");
  }

  if (pid == getpid()){ // to avoid taking a ckpt again on 'restart'
    if (lseek(ckptImage_fd, ckpt_h.context_off, SEEK_SET) == (off_t)-1) {
      ERROR("lseek()");
    }
    if (write(ckptImage_fd, &context, sizeof(context)) == -1) {
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
 
  memMaps_off      = ckpt_h.memMapsHeader_off;
  memMaps_data_off = ckpt_h.memMapsData_off;
  while (_readline(maps_fd, line) != -1) {
    if (is_vvar_line(line) || is_vdso_line(line) || is_vsyscall_line(line) ||
        is_skip_region(line)) {
      continue;
    }
    fill_memMap(&memMap, line, memMaps_data_off);
    if (memMap.v_addr > access_limit) {
      continue;
    }
    
    // write mem. map metadata to ckpt image
    if (lseek(ckptImage_fd, memMaps_off, SEEK_SET) == (off_t)-1) {
      ERROR("lseek()");
    }
    if (write(ckptImage_fd, &memMap, sizeof(memMap_t)) == -1) {
      ERROR("write()");
    }

    // write mem. map data to ckpt image
    if (lseek(ckptImage_fd, memMaps_data_off, SEEK_SET) == (off_t)-1) {
      ERROR("lseek()");
    }
    {
      int nread = 0;
      int read_total=0;
      while (read_total != memMap.size){
        if ((nread = write(ckptImage_fd, memMap.v_addr+read_total, 
              memMap.size-read_total)) == -1) {
          ERROR("write()");
        }
        read_total += nread;
      }
    }

    memMaps_off += sizeof(memMap_t);
    memMaps_data_off += memMap.size;
    memMaps_num++;
  }
  ckpt_h.memMapsNum = memMaps_num;
  
  if (close(maps_fd) == -1) {
    ERROR("close()");
  }
}
