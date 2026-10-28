#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
extern char *tzname[];
int main() {
  time_t vrem;
  struct tm *sp;

  vrem = time(NULL);

  setenv("TZ", "PST8", 1);
  tzset();

  sp = localtime(&vrem);
  printf("%d/%d/%02d %d:%02d %s\n", sp->tm_mon + 1, sp->tm_mday,
         sp->tm_year + 1900, sp->tm_hour, sp->tm_min, "America/Los_Angeles");
  return 0;
}
