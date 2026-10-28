#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

void steps12(char *param_1)

{
  int param2;
  int param1;
  FILE *stream;
  
  param2 = geteuid();
  param1 = getuid();
  printf("Real ID: %d\nEffective ID: %d\n", param1, param2);
  stream = fopen(param_1, "r");
  if (stream == NULL)
    perror("File open failed\n");
  else {
    fclose(stream);
    puts("Closed");
  }
  return;
}

int main(int argc, char *argv[]) {
  int uid;
  int iVar1;
  
  steps12(argv[1]);
  uid = getuid();
  iVar1 = setuid(uid);
  steps12(argv[1]);
  return 0;
}
