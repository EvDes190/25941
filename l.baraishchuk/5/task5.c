#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LINES 1000
#define MAX_LEN 1024

int main(int argc, char *argv[])
{
    int fd;
    int nlines = 0;
    int num;
    int n;
    int i;

    char c;
    char buf[MAX_LEN + 1];
    char input[100];

    off_t pos[MAX_LINES];
    int len[MAX_LINES];

    off_t start = 0;
    off_t cur;

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

    while (1) {

        printf("line number: ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

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