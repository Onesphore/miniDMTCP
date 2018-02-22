#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>

int main(int argc, char *argv[])
{
  int i = 0;
  while(1)
  {
    printf("%d.. ", i++);
    fflush(stdout);
    sleep(1);
  }


  exit(EXIT_SUCCESS);
}
