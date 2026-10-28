#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define MAX_LINES 1000

volatile sig_atomic_t timed_out = 0;

void alarm_handler(int sig)
{
    timed_out = 1;
}

int main(int argc, char *argv[])
{
    int fd;
    int nlines = 0;
    int num;
    int i;
    int first = 1;
    char extra;

    char input[100];
    char *data;

    off_t pos[MAX_LINES];
    int len[MAX_LINES];

    off_t start = 0;

    struct stat st;
    struct sigaction sa;

    if (argc != 2) {
        printf("usage: %s file\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    if (st.st_size == 0) {
        printf("empty file\n");
        close(fd);
        return 0;
    }

    data = mmap(NULL, st.st_size,
                PROT_READ, MAP_PRIVATE, fd, 0);

    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    for (i = 0; i < st.st_size; i++) {

        if (data[i] == '\n') {
            pos[nlines] = start;
            len[nlines] = i - start + 1;

            nlines++;
            start = i + 1;
        }
    }

    if (start < st.st_size) {
        pos[nlines] = start;
        len[nlines] = st.st_size - start;
        nlines++;
    }

    printf("Line   Offset   Length\n");

    for (i = 0; i < nlines; i++)
        printf("%4d   %6ld   %6d\n",
               i + 1, (long)pos[i], len[i]);

    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGALRM, &sa, NULL);

    while (1) {

        printf("line number: ");
        fflush(stdout);

        if (first) {
            timed_out = 0;
            alarm(5);
        }

        if (fgets(input, sizeof(input), stdin) == NULL) {

            if (first)
                alarm(0);

            if (timed_out) {
                printf("\nTime is over\n");

                fwrite(data, 1, st.st_size, stdout);

                first = 0;
                timed_out = 0;
            }

            clearerr(stdin);

            continue;
        }


        if (first) {
            alarm(0);
            first = 0;
        }

        if (sscanf(input, "%d %c", &num, &extra) != 1) {
            printf("enter a number\n");
            continue;
        }

        if (num == 0)
            break;

        if (num < 1 || num > nlines) {
            printf("no such line\n");
            continue;
        }

        fwrite(data + pos[num - 1],
            1,
            len[num - 1],
            stdout);
    }

    munmap(data, st.st_size);
    close(fd);

    return 0;
}