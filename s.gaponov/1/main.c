#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <ulimit.h>
#include <unistd.h>

extern char **environ;

void print_ids(void) {
    printf("Real user ID: %ld\n", (long)getuid());
    printf("Effective user ID: %ld\n", (long)geteuid());
    printf("Real group ID: %ld\n", (long)getgid());
    printf("Effective group ID: %ld\n", (long)getegid());
}

void print_process_ids(void) {
    printf("Process ID: %ld\n", (long)getpid());
    printf("Parent process ID: %ld\n", (long)getppid());
    printf("Process group ID: %ld\n", (long)getpgid(0));
}

int make_group_leader(void) {
    if (setpgid(0, 0) == -1) {
        perror("setpgid");
        return 1;
    }

    return 0;
}

int print_ulimit(void) {
    long limit = sysconf(_SC_CHILD_MAX);

    if (limit == -1) {
        perror("sysconf");
        return 1;
    }

    printf("Max user processes: %ld\n", limit);

    return 0;
}

int set_ulimit(const char *text) {
    char *end;
    long value = strtol(text, &end, 10);

    if (*text == '\0' || *end != '\0' || value < 0) {
        fprintf(stderr, "Invalid ulimit value\n");
        return 1;
    }

    if (ulimit(UL_SETFSIZE, value) == -1) {
        perror("ulimit");
        return 1;
    }

    printf("File size limit set to %ld blocks\n", value);

    return 0;
}

void print_core_limit(void) {
    struct rlimit limit;

    if (getrlimit(RLIMIT_CORE, &limit) == -1) {
        perror("getrlimit");
        return;
    }

    printf("Core file size: %llu bytes\n", (unsigned long long)limit.rlim_cur);
}

int set_core_limit(const char *text) {
    char *end;
    unsigned long long value = strtoull(text, &end, 10);

    if (*text == '\0' || *end != '\0' || text[0] == '-') {
        fprintf(stderr, "Invalid core size\n");
        return 1;
    }

    struct rlimit limit;

    if (getrlimit(RLIMIT_CORE, &limit) == -1) {
        perror("getrlimit");
        return 1;
    }

    limit.rlim_cur = (rlim_t)value;

    if (setrlimit(RLIMIT_CORE, &limit) == -1) {
        perror("setrlimit");
        return 1;
    }

    return 0;
}

void print_current_directory(void) {
    char path[1024];

    if (getcwd(path, sizeof(path)) == NULL) {
        perror("getcwd");
        return;
    }

    printf("Current directory: %s\n", path);
}

void print_environment(void) {
    for (int i = 0; environ[i] != NULL; i++) {
        printf("%s\n", environ[i]);
    }
}

int set_environment_variable(char *text) {
    if (putenv(text) != 0) {
        perror("putenv");
        return 1;
    }

    return 0;
}

int process_options(int argc, char *argv[]) {
    int option = getopt(argc, argv, "ispuU:cC:dvV:");
    char *argument = optarg;

    if (option == -1) {
        return 0;
    }

    int result = process_options(argc, argv);

    if (result != 0) {
        return result;
    }

    switch (option) {
    case 'i':
        print_ids();
        break;

    case 's':
        if (make_group_leader() != 0) {
            return 1;
        }
        break;

    case 'p':
        print_process_ids();
        break;

    case 'u':
        if (print_ulimit() != 0) {
            return 1;
        }
        break;

    case 'U':
        if (set_ulimit(argument) != 0) {
            return 1;
        }
        break;

    case 'c':
        print_core_limit();
        break;

    case 'C':
        if (set_core_limit(argument) != 0) {
            return 1;
        }
        break;

    case 'd':
        print_current_directory();
        break;

    case 'v':
        print_environment();
        break;

    case 'V':
        if (set_environment_variable(argument) != 0) {
            return 1;
        }
        break;

    default:
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) { return process_options(argc, argv); }