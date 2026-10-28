#include <sys/resource.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define PATH_SIZE 4097

char *options_text = "\n-i Type real and effective id of user and group.\n-s Process become new leader of group.\n-p Type id of process, process-parenth and group-process.\n-u Type ulimit vale.\n-Unew_ulimit Change ulimit value.\n-c Type size core-file in bytes, which may be created.\n-Csize Change size of core-file.\n-d Type current active directory.\n-v Type variables of environment and his values.\n-Vname=value Create new variable in environment or change value of exists variable.\n";

typedef struct arg {
    char opt;
    char *optarg;
} arg_t;

extern char **environ;

int main(int argc, char *argv[]) {
    arg_t args[argc];
    int args_count = 0;
    char path[PATH_SIZE];
    struct rlimit limit;
    unsigned long temp_limit;
    char *optbuf;
    char *equal_symbol;
    char *env;
	char *env_value;

    while((args[args_count].opt = getopt(argc, argv,"ispuU:cC:dvV:")) != -1) {
        args[args_count].optarg = optarg;
        args_count++;
    }

    for (int i = args_count - 1; i >= 0; i--) {
        switch(args[i].opt) {
            case 'C':
                temp_limit = atoi(args[i].optarg);
                printf("%llu\n", temp_limit);
                getrlimit(RLIMIT_CORE, &limit);
                if (temp_limit > limit.rlim_max)
                    limit.rlim_cur = limit.rlim_max;
                else
                    limit.rlim_cur = temp_limit;
                setrlimit(RLIMIT_CORE, &limit);
                getrlimit(RLIMIT_CORE, &limit);
                printf("%llu\n\n", limit.rlim_cur);
                break;
            case 'U':
                getrlimit(RLIMIT_FSIZE, &limit);
                temp_limit = atoi(args[i].optarg);
                limit.rlim_cur = temp_limit;

                if (setrlimit(RLIMIT_FSIZE, &limit) != -1)
                    printf("New ulimit: %llu\n\n", limit.rlim_cur);
                break;
            case 'V':
				optbuf = malloc(strlen(args[i].optarg) * sizeof(char));
				memcpy(optbuf, args[i].optarg, strlen(args[i].optarg) * sizeof(char));
				equal_symbol = strchr(optbuf, '=');
				assert(equal_symbol != NULL);
				*equal_symbol = '\0';
				env = optbuf;
				env_value = equal_symbol + 1;

				printf("%s = %s\n\n", env, env_value);
				assert(setenv(env, env_value, 1) == 0);
				break;
            case 'c':
                getrlimit(RLIMIT_CORE, &limit);
                printf("Core limit: ");
                if (limit.rlim_cur == RLIM_INFINITY)
                    puts("unlimited\n");
                else
                    printf("%u\n\n", limit.rlim_cur);
                break;
            case 'd':
                getcwd(path, PATH_SIZE - 1);
                printf("Current directory: %s\n\n", path);
                break;
            case 'i':
                printf("Real UID: %d\n", getuid());
                printf("Effective UID: %d\n", geteuid());
                printf("Real GID: %d\n", getgid());
                printf("Effective GID: %d\n\n", getegid());
                break;
            case 'p':
                printf("Process ID: %d\n", getpid());
                printf("Parent process ID: %d\n", getppid());
                printf("Process group ID: %d\n\n", getpgrp());
                break;
            case 's':
                setpgid(0,0);
                puts("Process is new group leader\n");
                break;
            case 'u':
                temp_limit = sysconf(2);
                printf("User processes limit: %lld\n\n", temp_limit);
                break;
            case 'v':
				for (char **i = environ; *i != NULL; i++)
					printf("%s\n", *i);
				printf("\n");
				break;
            default:
                printf("Invalid option\n");
                break;
        }
    }
    if (argc < 2)
        printf("%s", options_text);

    return 0;
}