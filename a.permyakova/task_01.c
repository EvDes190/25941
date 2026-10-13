#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

extern char **environ;

int main(int argc, char *argv[]) {
    char  opt_ch[64]; // символы опций
    char *opt_ar[64]; // аргументы опций 
    int   n = 0;
    int   flag;
    struct rlimit lim;

    while ((flag = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        opt_ch[n] = flag;
        opt_ar[n] = optarg;
        n++;
    }

    for (int i = n - 1; i >= 0; i--) {
        switch (opt_ch[i]) {
        case 'i':
            printf("UID=%d EUID=%d GID=%d EGID=%d\n", getuid(), geteuid(), getgid(), getegid());
            break;
        case 's':
            setpgid(0, 0);
            printf("PGID=%d\n", getpgrp());
            break;
        case 'p':
            printf("PID=%d PPID=%d PGID=%d\n", getpid(), getppid(), getpgrp());
            break;
        // размер файла
        case 'u':
            getrlimit(RLIMIT_NPROC, &lim);
            printf("ulimit=%llu\n", (unsigned long long)lim.rlim_cur);
            break;
        // изменение
        case 'U':
            getrlimit(RLIMIT_NPROC, &lim);
            lim.rlim_cur = lim.rlim_max = atol(opt_ar[i]);
            setrlimit(RLIMIT_NPROC, &lim);
            break;
        // размер файла создаваемого при падении
        case 'c':
            getrlimit(RLIMIT_CORE, &lim);
            printf("core=%llu\n", (unsigned long long)lim.rlim_cur);
            break;
        case 'C':
            getrlimit(RLIMIT_CORE, &lim);
            lim.rlim_cur = lim.rlim_max = atol(opt_ar[i]);
            setrlimit(RLIMIT_CORE, &lim);
            break;
        case 'd':
            ;	
            char buf[1024];
	    printf("cwd=%s\n", getcwd(buf, sizeof buf) ? buf : "(error)");
            break;
        case 'v':
            for (char **e = environ; *e; e++) printf("%s\n", *e);
            break;
        case 'V':
            putenv(opt_ar[i]);
            break;
        default:
            fprintf(stderr, "bad option: -%c\n", opt_ch[i]);
            return 1;
        }
    }
    return 0;
}
