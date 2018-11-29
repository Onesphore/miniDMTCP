#include <ckpt.hpp>
#include <utils/utils.hpp>

ucontext_t context;

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
  if (( ckptImage_fd = open("myckpt.ckpt", O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1) {
    ERROR("open");
  }
  ckpt_context();
  ckpt_memory();
//   regularFile _regularFile;
//   _regularFile.ckpt_fds();
}

void 
miniDMTCP::ckpt_context() {
  pid_t pid = getpid();
  memset(&context, 0, sizeof(context));
  if (getcontext(&context) == -1) {
    ERROR("getcontext()");
  }
  // to avoid taking a ckpt again on 'restart'
  if (pid == getpid()){
    if (write(ckptImage_fd, &context, sizeof(context)) == -1) {
      ERROR("write()");
    }
  }

}

void 
miniDMTCP::ckpt_memory() {
    int maps_fd;
    if ((maps_fd = open("/proc/self/maps", O_RDONLY)) == -1) {
      ERROR("open()");
    } 
    // leave space for the magic number, 'number_of_mem_sections':)
    if (lseek(ckptImage_fd, sizeof(int), SEEK_CUR) == (off_t)-1) {
      ERROR("lseek()");
    }

    char line[128];
    mem_section msection;
    int section_nbr = 0;
    void *access_limit = (void *)(long long)0xf000000000000000;

    while (_readline(maps_fd, line) != -1) {
      // check if it's any of the region we don't need to write
      // to the ckpt image file.
      if (is_vvar_line(line) || is_vdso_line(line) || is_vsyscall_line(line) ||
	  is_skip_region(line)) {
        continue;
      }

      fill_memsection(&msection, line);
      if (msection.address > access_limit) {
        continue;
      }
      section_nbr++;
      if (write(ckptImage_fd, &msection, sizeof(msection)) == -1) {
        ERROR("write()");
      }   
      
      int nread = 0;
      int read_total=0;
      while (read_total != msection.size){
        if ((nread = write(ckptImage_fd, msection.address+read_total, 
              msection.size-read_total)) == -1) {
	  ERROR("write()");
        }
        read_total += nread;
      }
    }

    if (lseek(ckptImage_fd, sizeof(context), SEEK_SET) == (off_t)-1) {
      ERROR("lseek()");
    } 
    // section_nbr: the number of memory sections written to 
    // the ckpt image file. Save this number also.
    if (write(ckptImage_fd, &section_nbr, sizeof(section_nbr)) == -1) {
      ERROR("write()");
    }

    if (close(maps_fd) == -1) {
      ERROR("close()");
    }
}
