#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <string.h>
#include <errno.h>

struct action {
    int   opt;
    char *arg;
};

static void print_ids(void)
{
    printf("-i: real uid=%d, effective uid=%d, real gid=%d, effective gid=%d\n",
           (int)getuid(),
           (int)geteuid(),
           (int)getgid(),
           (int)getegid());
}

static void become_group_leader(void)
{
    if (setpgid(0, 0) == -1) {
        perror("-s: setpgid");
    } else {
        printf("-s: process %d became leader of group %d\n",
               (int)getpid(), (int)getpgrp());
    }
}

static void print_process_info(void)
{
    printf("-p: pid=%d, ppid=%d, pgid=%d\n",
           (int)getpid(),
           (int)getppid(),
           (int)getpgrp());
}

static void print_cwd(void)
{
    char buf[4096];
    if (getcwd(buf, sizeof(buf)) != NULL) {
        printf("-d: cwd=%s\n", buf);
    } else {
        perror("-d: getcwd");
    }
}

static void print_environment(void)
{
    extern char **environ;
    printf("-v: environment variables:\n");
    for (char **e = environ; *e != NULL; ++e) {
        printf("    %s\n", *e);
    }
}

static void handle_limit(int resource, const char *tag, const char *newval)
{
    struct rlimit rl;

    if (getrlimit(resource, &rl) == -1) {
        perror(tag);
        return;
    }

    if (newval == NULL) {
        printf("%s: soft=%ld bytes, hard=%ld bytes\n",
               tag, (long)rl.rlim_cur, (long)rl.rlim_max);
        return;
    }

    errno = 0;
    char *end;
    long value = strtol(newval, &end, 10);

    if (errno == ERANGE || *end != '\0' || value < 0) {
        fprintf(stderr, "%s: invalid value '%s'\n", tag, newval);
        return;
    }

    rl.rlim_cur = (rlim_t)value;
    if (setrlimit(resource, &rl) == -1) {
        perror(tag);
    } else {
        printf("%s: value changed to %ld\n", tag, value);
    }
}

static void set_environment_variable(const char *arg)
{
    if (arg == NULL || strchr(arg, '=') == NULL) {
        fprintf(stderr, "-V: expected format name=value\n");
        return;
    }

    if (putenv(strdup(arg)) != 0) {
        perror("-V: putenv");
    } else {
        printf("-V: set %s\n", arg);
    }
}

int main(int argc, char *argv[])
{
    struct action actions[128];
    int count = 0;
    int opt;

    if (argc == 1 ||
        strcmp(argv[1], "help")   == 0 ||
        strcmp(argv[1], "-h")     == 0) {

        printf("Usage: %s [options]\n"
               "\n"
               "-i             Print real and effective user and group IDs.\n"
               "-s             Make the process a group leader. See setpgid(2).\n"
               "-p             Print process, parent process and process group IDs.\n"
               "-u             Print the ulimit value.\n"
               "-Unew_ulimit   Change the ulimit value. See strtol(3C).\n"
               "-c             Print the size in bytes of the core file that can be created.\n"
               "-Csize         Change the core file size.\n"
               "-d             Print the current working directory.\n"
               "-v             Print environment variables and their values.\n"
               "-Vname=value   Add a new variable to the environment or change an existing one.\n",
               argv[0]);
        return 0;
    }

    opterr = 0;

    while ((opt = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        if (opt == '?') {
            fprintf(stderr, "Unknown option: -%c\n", optopt);
            continue;
        }
        if (count < (int)(sizeof(actions) / sizeof(actions[0]))) {
            actions[count].opt = opt;
            actions[count].arg = optarg;
            count++;
        }
    }

    for (int i = count - 1; i >= 0; --i) {
        switch (actions[i].opt) {
            case 'i': print_ids();                                      break;
            case 's': become_group_leader();                            break;
            case 'p': print_process_info();                             break;
            case 'u': handle_limit(RLIMIT_FSIZE, "-u", NULL);           break;
            case 'U': handle_limit(RLIMIT_FSIZE, "-U", actions[i].arg); break;
            case 'c': handle_limit(RLIMIT_CORE,  "-c", NULL);           break;
            case 'C': handle_limit(RLIMIT_CORE,  "-C", actions[i].arg); break;
            case 'd': print_cwd();                                      break;
            case 'v': print_environment();                              break;
            case 'V': set_environment_variable(actions[i].arg);         break;
        }
    }

    if (optind < argc) {
        printf("Unprocessed arguments:");
        for (int i = optind; i < argc; ++i) {
            printf(" %s", argv[i]);
        }
        putchar('\n');
    }

    return 0;
}
