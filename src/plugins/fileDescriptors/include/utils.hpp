#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_FD_NUM 256 // FIXME: should set this value to the actual number

class fileDescriptor {
public:
  fileDescriptor(): fds(NULL), fds_num(0) {
    strcpy(fds_path, "/proc/self/fd/");
  }

  struct fd_t {
    int fdno;
    char name[256];
    char type;
    char permissions[3];  // rwx
    off_t offset;
  };

  fd_t* fds;
  int fds_num;
  char fds_path[256];

public:
  void getFileDescriptors();
};
