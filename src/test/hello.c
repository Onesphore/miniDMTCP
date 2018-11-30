#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int main() {
  int fd = open("hello.txt", O_RDONLY);
  const struct timespec req = {.tv_sec=1, .tv_nsec=0};
  int i=1;
  char *buf = malloc(64);;
  size_t size = 64;
  while(1) {
    read(fd, buf, 8);
    printf("%d.. ", i++);
    printf("%s\n", buf);
    fflush(stdout);
    nanosleep(&req, NULL);
    if (i == 7)
      break;
  }
  printf("inside the 'for' loop ...\n");
  for (;; ++i) {
    read(fd, buf, 7);
    printf("%d.. ", i);
    printf("%s\n", buf);
    fflush(stdout);
    nanosleep(&req, NULL);
  }
  free(buf);
  close(fd);

  return 0;
}
