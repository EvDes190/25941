#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int shag1_2() {
  unsigned long uid = getuid();   
  unsigned long euid = geteuid();

  printf("getuid  = %lu, geteuid = %lu \n", uid, euid);
  FILE *in = fopen("a", "r");
  if (!in) {
    perror("fopen");
    return 1;
  }
  fclose(in);
  return 0;
}
int main() {
  if (shag1_2())
    return 1;

  if (getuid() != geteuid()) {
    if (setuid(getuid()) == -1) {
      perror("setuid");
      return 1;
    }
  }
  if (shag1_2())
    return 1;

  return 0;
}
