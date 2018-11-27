#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>

int main(int argc, char *argv[])
{
  const struct timespec req = {.tv_sec=1, .tv_nsec=0};
  int i = 0;
  while(1)
  {
    printf("%d.. ", i++);
    fflush(stdout);
    nanosleep(&req, NULL);
  }


  exit(EXIT_SUCCESS);
}
