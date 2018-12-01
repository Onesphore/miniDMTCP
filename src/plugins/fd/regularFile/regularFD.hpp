#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common.hpp"

#define MAX_FD_NUM 256 // FIXME: should set this value to the actual number

class regularFD: virtual public fileDescriptor {
public:
  void writeFdToCkptImg(int ckptImgFd, off_t offset, fd_t fd_metadata);
};

// FIXME: this class should be in its own file
class socketFD: virtual public fileDescriptor {
public:
  void writeFdToCkptImg(int ckptImgFd, off_t offset, fd_t fd_metadata);
};

// FIXME: this class should be in its own file
class ptyFD: virtual public fileDescriptor {
public:
  void writeFdToCkptImg(int ckptImgFd, off_t offset, fd_t fd_metadata);
};
