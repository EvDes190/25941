#include <unistd.h>
#include <stdio.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define PATH_SIZE 4097

extern char **environ;

int main(int argc, char *argv[]) {
	int opt;
	struct rlimit limit;
	char *endptr;
	unsigned long temp_limit;
	char path[PATH_SIZE];
	char *optbuf;
	char *env;
	char *env_value;
	char *equal_symbol;
	while ((opt = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
		switch (opt) {
			case 'i':
				printf("Real UID: %d\n", getuid());
				printf("Effective UID: %d\n", geteuid());
				printf("Real GID: %d\n", getgid());
				printf("Effective GID: %d\n\n", getegid());
				break;
			case 's':
				setpgid(0, 0);
				printf("Process is new group leader\n\n");
				break;
			case 'p':
				printf("Process ID: %d\n", getpid());
				printf("Parent process ID: %d\n", getppid());
				printf("Process group ID: %d\n\n", getpgrp());
				break;
			case 'u':
				assert(getrlimit(RLIMIT_STACK, &limit) == 0);
				printf("Soft limit: %ul\n", limit.rlim_cur);
				printf("Hard limit: %ul\n\n", limit.rlim_max);
			case 'U':
				temp_limit = strtol(optarg, &endptr, 10);
				assert(*endptr == '\0' && "U: Invalid value\n\n");
				assert(getrlimit(RLIMIT_STACK, &limit) == 0);
				limit.rlim_max = temp_limit;
				limit.rlim_cur = temp_limit < limit.rlim_cur ? temp_limit : limit.rlim_cur;
				assert(setrlimit(RLIMIT_STACK, &limit) == 0);
				printf("%d\n\n", limit.rlim_max);
				break;
			case 'c':
				assert(getrlimit(RLIMIT_CORE, &limit) == 0);
				printf("Soft core limit: %ul\n", limit.rlim_cur);
				printf("Hard core limit: %ul\n\n", limit.rlim_max);
				break;
			case 'C':
				temp_limit = strtol(optarg, &endptr, 10);
                                assert(*endptr == '\0' && "U: Invalid value\n\n");
                                assert(getrlimit(RLIMIT_CORE, &limit) == 0);
                                limit.rlim_max = temp_limit;
                                limit.rlim_cur = temp_limit < limit.rlim_cur ? temp_limit : limit.rlim_cur;
                                assert(setrlimit(RLIMIT_CORE, &limit) == 0);
                                printf("%d\n\n", limit.rlim_max);
				break;
			case 'd':
				assert(getcwd(path, PATH_SIZE) != NULL);
				printf("Current directory: %s\n\n", path);
				break;
			case 'v':
				for (char **i = environ; *i != NULL; i++)
					printf("%s\n", *i);
				printf("\n");
				break;
			case 'V':
				optbuf = malloc(strlen(optarg) * sizeof(char));
				memcpy(optbuf, optarg, strlen(optarg) * sizeof(char));
				equal_symbol = strchr(optbuf, '=');
				assert(equal_symbol != NULL);
				*equal_symbol = '\0';
				env = optbuf;
				env_value = equal_symbol + 1;

				printf("%s = %s\n\n", env, env_value);
				assert(setenv(env, env_value, 1) == 0);
				break;
			default:
				fprintf(stderr, "Invalid option \n");
				break;
		}
	}
}
