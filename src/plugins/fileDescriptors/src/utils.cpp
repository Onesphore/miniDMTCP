#include <utils.hpp>
#include <iostream> // FIXME: remove it, was used for debugging

void fileDescriptor::getFileDescriptors() {
  fds = (fd_t*) malloc(MAX_FD_NUM * sizeof(fd_t));
  fds_num = 0;
  DIR *dir;
  struct dirent *dirEntry;          // directory entry
  dir = opendir(fds_path);

  char path[256];
  strcpy(path, fds_path);

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
      readlink(path, fds[fds_num].name, 256);
      fds[fds_num].fdno = atoi(dirEntry->d_name);
      fds[fds_num].offset = lseek(atoi(dirEntry->d_name), 0, SEEK_SET);
      fds_num++;
    }
    closedir(dir);
  }
}
