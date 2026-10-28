#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <ulimit.h>
#include <unistd.h>
extern char **environ;

struct opt_rec {
  int opt;
  char *arg;
};
struct opt_rec saved[256];
int save = 0;
void do_i() {
  printf("uid = %d euid = %d gid = %d egid = %d\n", getuid(), geteuid(),
         getgid(), getegid());
}
void do_p() {
  printf("pid = %d ppid = %d pgid = %d\n", getpid(), getppid(), getpgrp());
}
void do_d() {
  char buf[4096];
  if (getcwd(buf, sizeof buf))
    printf("cwd = %s\n", buf);
}
void do_s() {
  if (setpgid(0, 0) != -1)
    printf("pgid = %d\n", getpgrp());
}
void do_u() {
    printf("ulimit = %ld\n", sysconf(_SC_CHILD_MAX));
}
void do_U(const char *arg) {
  struct rlimit rl;
  getrlimit(RLIMIT_FSIZE, &rl);
  rl.rlim_cur = (rlim_t)atoi(arg);
  if (setrlimit(RLIMIT_FSIZE, &rl) != -1)
    printf("new ulimit =  %llu\n", (unsigned long long)rl.rlim_cur);
}
void do_c() {
  struct rlimit rl;
  getrlimit(RLIMIT_CORE, &rl);
  printf("core size = %llu\n", (unsigned long long)rl.rlim_cur);
}
void do_C(const char *arg) {
  struct rlimit rl;
  getrlimit(RLIMIT_CORE, &rl);
  rl.rlim_cur = (rlim_t)atol(arg);
  if (setrlimit(RLIMIT_CORE, &rl) != -1)
    printf("new core size = %llu\n", (unsigned long long)rl.rlim_cur);
}
void do_v() {
  for (char **e = environ; *e; ++e)
    printf("%s\n", *e);
}

void do_V(const char *arg) {
  char *eq = strchr(arg, '=');
  if (!eq)
    return;
  *eq = '\0';
  if (setenv(arg, eq + 1, 1) != -1)
    printf("set %s = %s\n", arg, eq + 1);
  *eq = '=';
}
int main(int argc, char *argv[]) {

  int opt;
  while ((opt = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
    saved[save].opt = opt;
    saved[save].arg = optarg;
    save++;
  }
  for (int i = save - 1; i >= 0; --i) {
    switch (saved[i].opt) {
    case 'i':
      do_i();
      break;
    case 'p':
      do_p();
      break;
    case 'd':
      do_d();
      break;
    case 's':
      do_s();
      break;
    case 'u':
      do_u();
      break;
    case 'U':
      do_U(saved[i].arg);
      break;
    case 'c':
      do_c();
      break;
    case 'C':
      do_C(saved[i].arg);
      break;
    case 'v':
      do_v();
      break;
    case 'V':
      do_V(saved[i].arg);
      break;
    default:
      fprintf(stderr, "not -%c\n", optopt);
    }
  }
  return 0;
}
