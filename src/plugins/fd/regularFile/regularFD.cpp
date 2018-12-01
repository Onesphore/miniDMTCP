// #include <utils.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "regularFD.hpp"

void
regularFD::writeFdToCkptImg(int ckptImgFd, off_t offset, fd_t fd_metadata) {
  int fds_num;                  // the number of file descriptors
  fd_t* fds;                    // pointer to file descriptor metadata

  fds = (fd_t*) malloc(MAX_FD_NUM * sizeof(fd_t));
  getFileDescriptors(&fds, &fds_num);
  for (int i=0; i<fds_num; ++i) {
    if (write(ckptImgFd, &fds[i], sizeof(fd_t)) == -1) {
      ERROR("write()");
    }
  }

  free(fds);
}

void
socketFD::writeFdToCkptImg(int ckptImgFd, off_t offset, fd_t fd_metadata) {
  int fds_num;                  // the number of file descriptors
  fd_t* fds;                    // pointer to file descriptor metadata

  fds = (fd_t*) malloc(MAX_FD_NUM * sizeof(fd_t));
  getFileDescriptors(&fds, &fds_num);
  for (int i=0; i<fds_num; ++i) {
    if (write(ckptImgFd, &fds[i], sizeof(fd_t)) == -1) {
      ERROR("write()");
    }
  }

  free(fds);
}

void
ptyFD::writeFdToCkptImg(int ckptImgFd, off_t offset, fd_t fd_metadata) {
  int fds_num;                  // the number of file descriptors
  fd_t* fds;                    // pointer to file descriptor metadata

  fds = (fd_t*) malloc(MAX_FD_NUM * sizeof(fd_t));
  getFileDescriptors(&fds, &fds_num);
  for (int i=0; i<fds_num; ++i) {
    if (write(ckptImgFd, &fds[i], sizeof(fd_t)) == -1) {
      ERROR("write()");
    }
  }

  free(fds);
}
