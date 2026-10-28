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
    int bad;
};

struct opt_rec saved[256];
int save = 0;


void do_h()
{
    printf(
        "Usage: ./task1 [options]\n"
        "  -i        print user/group ids\n"
        "  -s        become process group leader\n"
        "  -p        print process ids\n"
        "  -u        print ulimit\n"
        "  -U1000    set ulimit to 1000\n"
        "  -c        print core size\n"
        "  -C1000    set core size to 1000\n"
        "  -d        print current directory\n"
        "  -v        print environment\n"
        "  -Vx=y     set environment variable\n"
        "  -h        help\n"
        "\n"
        "Example:\n"
        "  ./task1 -v -VTEST=hello\n"
        "  TEST=hello will be visible in environment\n"
        "\n"
        "Options work right to left\n"
    );
}


void do_i()
{
    printf("uid = %d euid = %d gid = %d egid = %d\n",
           getuid(), geteuid(), getgid(), getegid());
}


void do_p()
{
    printf("pid = %d ppid = %d pgid = %d\n",
           getpid(), getppid(), getpgrp());
}


void do_d()
{
    char buf[4096];

    if (getcwd(buf, sizeof(buf)))
        printf("cwd = %s\n", buf);
}


void do_s()
{
    if (setpgid(0, 0) != -1)
        printf("pgid = %d\n", getpgrp());
}


void do_u()
{
    printf("ulimit = %ld\n", sysconf(_SC_CHILD_MAX));
}


void do_U(const char *arg)
{
    struct rlimit rl;

    getrlimit(RLIMIT_FSIZE, &rl);

    rl.rlim_cur = (rlim_t)atoi(arg);

    if (setrlimit(RLIMIT_FSIZE, &rl) != -1)
        printf("new ulimit = %llu\n",
               (unsigned long long)rl.rlim_cur);
}


void do_c()
{
    struct rlimit rl;

    getrlimit(RLIMIT_CORE, &rl);

    printf("core size = %llu\n",
           (unsigned long long)rl.rlim_cur);
}


void do_C(const char *arg)
{
    struct rlimit rl;

    getrlimit(RLIMIT_CORE, &rl);

    rl.rlim_cur = (rlim_t)atol(arg);

    if (setrlimit(RLIMIT_CORE, &rl) != -1)
        printf("new core size = %llu\n",
               (unsigned long long)rl.rlim_cur);
}


void do_v()
{
    char **e;

    for (e = environ; *e; ++e)
        printf("%s\n", *e);
}


void do_V(char *arg)
{
    char *eq;

    eq = strchr(arg, '=');

    if (!eq)
        return;

    *eq = '\0';

    if (setenv(arg, eq + 1, 1) != -1)
        printf("set %s = %s\n", arg, eq + 1);

    *eq = '=';
}


int main(int argc, char *argv[])
{
    int opt;
    int i;

    opterr = 0;

    while ((opt = getopt(argc, argv, "hispuU:cC:dvV:")) != -1) {

        if (save >= 256) {
            printf("too many options\n");
            return 1;
        }

        saved[save].opt = opt;
        saved[save].arg = optarg;
        saved[save].bad = optopt;

        save++;
    }

    /* выполняем опции справа налево */
    for (i = save - 1; i >= 0; --i) {

        switch (saved[i].opt) {

        case 'h':
            do_h();
            break;

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

        case '?':
            fprintf(stderr, "not -%c\n", saved[i].bad);
            break;
        }
    }

    return 0;
}