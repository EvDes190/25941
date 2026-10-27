#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

extern char **environ;

int parse_number(const char *str, long *value)
{
    char *end;
    long number;

    if (str == NULL || *str == '\0')
    {
        return 0;
    }

    errno = 0;
    end = NULL;

    number = strtol(str, &end, 10);

    if (errno != 0 || end == str || *end != '\0')
    {
        return 0;
    }

    if (number < 0)
    {
        return 0;
    }

    *value = number;

    return 1;
}

void print_ids(void)
{
    printf("Real UID: %d\n", (int)getuid());
    printf("Effective UID: %d\n", (int)geteuid());

    printf("Real GID: %d\n", (int)getgid());
    printf("Effective GID: %d\n", (int)getegid());
}

void print_process_ids(void)
{
    printf("PID: %d\n", (int)getpid());
    printf("PPID: %d\n", (int)getppid());
    printf("PGID: %d\n", (int)getpgrp());
}

int make_group_leader(void)
{
    if (setpgid(0, 0) == -1)
    {
        perror("setpgid");
        return 0;
    }

    printf("Process became a process group leader\n");
    printf("PID: %d\n", (int)getpid());
    printf("PGID: %d\n", (int)getpgrp());

    return 1;
}

/*
 * Solaris не поддерживает RLIMIT_NPROC.
 * Максимальное число процессов пользователя (maxuprc) читается
 * через sysconf(_SC_CHILD_MAX) — то же, что показывает "ulimit -u".
 */
int print_ulimit(void)
{
    long max_proc;

    errno = 0;
    max_proc = sysconf(_SC_CHILD_MAX);

    if (max_proc == -1)
    {
        perror("sysconf");
        return 0;
    }

    printf("ulimit: %ld\n", max_proc);

    return 1;
}

/*
 * На Solaris изменить лимит процессов пользователя из программы нельзя.
 * maxuprc задаётся в /etc/system и требует перезагрузки.
 * Поэтому -U сообщает об этом и завершается с ошибкой.
 */
int change_ulimit(const char *str)
{
    long value;

    if (!parse_number(str, &value))
    {
        fprintf(stderr, "Invalid value for -U: %s\n", str);
        return 0;
    }

    fprintf(stderr,
            "Changing the maximum number of user processes is not "
            "supported on Solaris from a program.\n"
            "Edit /etc/system (set maxuprc=%ld) and reboot.\n",
            value);

    return 0;
}

int print_core_size(void)
{
    struct rlimit limit;

    if (getrlimit(RLIMIT_CORE, &limit) == -1)
    {
        perror("getrlimit");
        return 0;
    }

    if (limit.rlim_cur == RLIM_INFINITY)
        printf("Core file size: unlimited\n");
    else
        printf("Core file size: %llu bytes\n",
               (unsigned long long)limit.rlim_cur);

    return 1;
}

int change_core_size(const char *str)
{
    long value;
    struct rlimit limit;

    if (!parse_number(str, &value))
    {
        fprintf(stderr, "Invalid value for -C: %s\n", str);
        return 0;
    }

    if (getrlimit(RLIMIT_CORE, &limit) == -1)
    {
        perror("getrlimit");
        return 0;
    }

    limit.rlim_cur = (rlim_t)value;

    if (setrlimit(RLIMIT_CORE, &limit) == -1)
    {
        perror("setrlimit");
        return 0;
    }

    printf("Core file size changed to %ld bytes\n", value);

    return 1;
}

int print_directory(void)
{
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd");
        return 0;
    }

    printf("Current directory: %s\n", cwd);

    return 1;
}

void print_environment(void)
{
    char **env = environ;

    while (*env != NULL)
    {
        printf("%s\n", *env);
        env++;
    }
}

int change_environment(const char *argument)
{
    char *copy;
    char *equal;

    if (argument == NULL)
    {
        fprintf(stderr, "Missing environment variable\n");
        return 0;
    }

    copy = malloc(strlen(argument) + 1);

    if (copy == NULL)
    {
        perror("malloc");
        return 0;
    }

    strcpy(copy, argument);
    equal = strchr(copy, '=');

    if (equal == NULL)
    {
        fprintf(stderr,
                "Invalid environment variable format: %s\n",
                argument);

        free(copy);
        return 0;
    }

    *equal = '\0';

    if (copy[0] == '\0')
    {
        fprintf(stderr, "Environment variable name is empty\n");

        free(copy);
        return 0;
    }

    if (setenv(copy, equal + 1, 1) == -1)
    {
        perror("setenv");
        free(copy);
        return 0;
    }

    printf("Environment variable changed: %s=%s\n",
           copy, equal + 1);

    free(copy);

    return 1;
}

int main(int argc, char *argv[])
{
    int opt;

    if (argc == 1)
    {
        printf("Input command:\n"
               "-i  Печатает реальные и эффективные идентификаторы пользователя и группы.\n"
               "-s  Процесс становится лидером группы. Подсказка: смотри setpgid(2).\n"
               "-p  Печатает идентификаторы процесса, процесса-родителя и группы процессов.\n"
               "-u  Печатает значение ulimit\n"
               "-Unew_ulimit  Изменяет значение ulimit. Подсказка: смотри atol(3C) на странице руководства strtol(3C)\n"
               "-c  Печатает размер в байтах core-файла, который может быть создан.\n"
               "-Csize  Изменяет размер core-файла\n"
               "-d  Печатает текущую рабочую директорию\n"
               "-v  Распечатывает переменные среды и их значения\n"
               "-Vname=value  Вносит новую переменную в среду или изменяет значение существующей переменной.\n");
    }

    while ((opt = getopt(argc, argv, "ispuU:cC:dvV:")) != -1)
    {
        switch (opt)
        {
            case 'i':
                print_ids();
                break;

            case 's':
                if (!make_group_leader())
                {
                    return 1;
                }
                break;

            case 'p':
                print_process_ids();
                break;

            case 'u':
                if (!print_ulimit())
                {
                    return 1;
                }
                break;

            case 'U':
                if (!change_ulimit(optarg))
                {
                    return 1;
                }
                break;

            case 'c':
                if (!print_core_size())
                {
                    return 1;
                }
                break;

            case 'C':
                if (!change_core_size(optarg))
                {
                    return 1;
                }
                break;

            case 'd':
                if (!print_directory())
                {
                    return 1;
                }
                break;

            case 'v':
                print_environment();
                break;

            case 'V':
                if (!change_environment(optarg))
                {
                    return 1;
                }
                break;

            case '?':
            default:
                fprintf(stderr, "Unknown option\n");
                return 1;
        }
    }

    if (optind < argc)
    {
        fprintf(stderr, "Unexpected argument: %s\n", argv[optind]);
        return 1;
    }

    return 0;
}
