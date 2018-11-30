#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_FD_NUM 256 // FIXME: should set this value to the actual number

// This class will be inherited by each type of file descriptor
class fileDescriptor {
public:
  // a struct to store the metadata of a file descriptor
  struct fd_t {
    int fdno;             // file descriptor
    char name[256];       // the file name: 'readlink(/proc/self/fd/x)'
    char type;            // FIXME: do we actually need this field
    char permissions[3];  // FIXME: there must be a better way to encode permissions
    off_t offset;         // the offset at the time of ckpting
  };

  fd_t* fds;              // FIXME: this variable should be outside this class
  int fds_num;            // FIXME: this variable should not be here
  char fds_path[256];     // FIXME: this variable should not be here 

protected:
  // When this function returns 'fds' will be pointing
  // to the list of file descriptor metadata of all the file descriptors 
  // in the process.
  // It returns the number of file descriptors.
  virtual int getFileDescriptors(fd_t* fds)=0;

public:
  // This functions writes to the ckpt image, 'ckptImgFd', file descriptor
  // metadata pointed to by 'fds'.
  // 'num', the number of file descriptor metadata
  virtual void writeFdsToCkptImg(int ckptImgFd, fd_t* fds, int num)=0;
};
