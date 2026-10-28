#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINES 1000
#define MAX_LEN 1024

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
    int n;
    int i;
    int first = 1;

    char c;
    char buf[MAX_LEN + 1];
    char input[100];

    off_t pos[MAX_LINES];
    int len[MAX_LINES];

    off_t start = 0;
    off_t cur;

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

    while (read(fd, &c, 1) == 1) {

        if (c == '\n') {
            cur = lseek(fd, 0L, SEEK_CUR);

            pos[nlines] = start;
            len[nlines] = cur - start;

            nlines++;
            start = cur;
        }
    }

    cur = lseek(fd, 0L, SEEK_CUR);

    if (cur > start) {
        pos[nlines] = start;
        len[nlines] = cur - start;
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

                lseek(fd, 0L, SEEK_SET);

                while ((n = read(fd, buf, MAX_LEN)) > 0)
                    write(1, buf, n);
            }

            break;
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

        if (len[num - 1] > MAX_LEN) {
            printf("line too long\n");
            continue;
        }

        lseek(fd, pos[num - 1], SEEK_SET);

        n = read(fd, buf, len[num - 1]);

        if (n == -1) {
            perror("read");
            break;
        }

        buf[n] = '\0';
        printf("%s", buf);
    }

    close(fd);
    return 0;
}