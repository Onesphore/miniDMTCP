#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main() {
  const struct timespec req = {.tv_sec=1, .tv_nsec=0};
  int i=0;
  while(1) {
    printf("%d.. ", i++);
    fflush(stdout);
    nanosleep(&req, NULL);
  }
  
  return 0;
}
