#ifndef FD_UTILS_COMMON_HPP
#define FD_UTILS_COMMON_HPP

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#define ERROR(msg)    \
do {                  \
  perror(msg);        \
  exit(EXIT_FAILURE); \
} while(0);


#define MAX_FD_NUM 256 // FIXME: should set this value to the actual number

// FIXME: the structure of the class below must be redesigned.

// enum fileDescr_type {
//   REGULAR = 1,
//   SOCKET,
//   PTY
// };

// struct fd_t {
//   int            fd_no;
//   char           fd_name[256];
//   fileDescr_type fd_type;
//   bool           fd_r;
//   bool           fd_w;
//   bool           fd_x;
//   off_t          offset;
// };

// This class will be inherited by each type of file descriptor
class fileDescriptor {
public:
  static void getFileDescriptors(fd_t** fds_ptr, int* fds_num_ptr);
  virtual void writeFdToCkptImg(int, off_t, fd_t)=0;
};

// FIXME: should not be in the header file
void
fileDescriptor::getFileDescriptors(fd_t** fds_ptr, int* fds_num_ptr) {
  const char* fds_path = "/proc/self/fd/";
  int fds_num = 0;
  DIR *dir;
  struct dirent *dirEntry;          // directory entry
  char path[256];
  
  // FIXME: first count the number of 'fds' and then allocate memory
  *fds_ptr = (fd_t*) malloc(128 * sizeof(fd_t));

  strcpy(path, fds_path);
  dir = opendir(fds_path);
  if (dir) {
    while ((dirEntry = readdir(dir)) != NULL) {
      if (strcmp(dirEntry->d_name, ".") == 0) {
        continue;
      }
      if (strcmp(dirEntry->d_name, "..") == 0) {
        continue;
      }
      if ((uint32_t)atoi(dirEntry->d_name) != 3) { // skip stdin, stdout, stderr
        continue;
      }
      strcpy(path+strlen(fds_path), dirEntry->d_name);
      if (readlink(path, (*fds_ptr)[fds_num].fd_name, 256) == -1) {
        ERROR("readlink()");
      }
      (*fds_ptr)[fds_num].fd_no = atoi(dirEntry->d_name);
      if (((*fds_ptr)[fds_num].offset = 
	   lseek(atoi(dirEntry->d_name), 0, SEEK_SET)) == (off_t)-1) {
        ERROR("lseek()");
      }
      (*fds_ptr)[fds_num].fd_type = REGULAR;
      fds_num++;
    }
    closedir(dir);
  } else {
    ERROR("opendir()");
  }

  *fds_num_ptr = fds_num;
}

#endif //  FD_UTILS_COMMON_HPP
