#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common.hpp"

#define MAX_FD_NUM 256 // FIXME: should set this value to the actual number

class regularFD: virtual public fileDescriptor {
private:
  virtual int getFileDescriptors(fd_t* fds);

public:
  virtual void writeFdsToCkptImg(int ckptImgFd, fd_t* fds, int num);
}
