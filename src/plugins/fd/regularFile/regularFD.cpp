// #include <utils.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "regularFD.hpp"

int
regularFD::getFileDescriptors(fd_t* fds) {
  const char* fds_path = "/proc/self/fd/";
  int fds_num = 0;
  DIR *dir;
  struct dirent *dirEntry;          // directory entry
  char path[256];

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
      if ((uint32_t)atoi(dirEntry->d_name) < 3) { // skip stdin, stdout, stderr
        continue;
      }
      strcpy(path+strlen(fds_path), dirEntry->d_name);
      if (readlink(path, fds[fds_num].name, 256) == -1) {
        ERROR("readlink()");
      }
      fds[fds_num].fdno = atoi(dirEntry->d_name);
      if ((fds[fds_num].offset = 
	   lseek(atoi(dirEntry->d_name), 0, SEEK_SET)) == (off_t)-1) {
        ERROR("lseek()");
      }
      fds_num++;
    }
    closedir(dir);
  } else {
    ERROR("opendir()");
  }

  return fds_num;
}

int
regularFD::writeFdsToCkptImg(int ckptImgFd) {
  int fds_num;                  // the number of file descriptors
  fd_t* fds;                    // pointer to file descriptor metadata

  fds = (fd_t*) malloc(MAX_FD_NUM * sizeof(fd_t));
  fds_num = getFileDescriptors(fds);
  for (int i=0; i<fds_num; ++i) {
    if (write(ckptImgFd, &fds[i], sizeof(fd_t)) == -1) {
      ERROR("write()");
    }
  }

  free(fds);

  return fds_num;
}
