#include <utils.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// FIXME: handle errors

class regularFile: virtual fileDescriptor {
public:
  void ckpt_fds();
}

void regularFile::ckpt_fds() {
  // FIXME: make it more robust
  getFileDescriptors();
  // store the file desriptors to the ckpt image
  for (int i=0; i<fds_num; ++i) {
    write(miniDMTCP::ckptImage_fd, fds[i], sizeof(fds[i]));
  }
}
