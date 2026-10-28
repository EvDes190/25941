#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>

typedef struct Line {
    int len_line;
    int offset;
} Line;

typedef struct Array {
    int len;
    int v;
    Line *list;
} Array;

static sigjmp_buf timeout_jmp;

void popLine(Array *lines, int len_line, int offset) {
    if (lines->len == lines->v) {
        lines->list = (Line *)realloc(lines->list, sizeof(Line) * lines->v * 2);
        lines->v *= 2;
    }
    lines->list[lines->len].len_line = len_line;
    lines->list[lines->len++].offset = offset;
}

void on_alarm(int sig) {
    (void)sig;
    siglongjmp(timeout_jmp, 1);
}

void printAll(int f) {
    if (lseek(f, 0, SEEK_SET) == -1) {
        perror("lseek");
        return;
    }
    char buf[4096];
    ssize_t n;
    while ((n = read(f, buf, sizeof(buf))) > 0) {
        if (write(STDOUT_FILENO, buf, n) != n) {
            perror("write");
            return;
        }
    }
    if (n < 0) perror("read");
}

int readNumber(int *idx) {
    if (scanf("%d", idx) != 1) {
        printf("invalid input\n");
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        return 0;
    }
    return 1;
}

int readNumberWithTimeout(int *idx) {
    if (sigsetjmp(timeout_jmp, 1) != 0) {
        return 0;
    }

    struct sigaction sa;
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    alarm(5);

    int ok = readNumber(idx);

    alarm(0);

    return ok;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("The file name has not been passed\n");
        return 1;
    }

    Array lines;
    lines.len = 0;
    lines.v = 20;
    lines.list = (Line*)malloc(sizeof(Line) * lines.v);

    int f = open(argv[1], O_RDONLY);
    if (f == -1) {
        perror("open");
        free(lines.list);
        return 1;
    }

    char c;
    int line_start = 0;
    int cur_offset = 0;

    while (read(f, &c, 1) == 1) {
        cur_offset += 1;
        if (c == '\n') {
            popLine(&lines, cur_offset - line_start, line_start);
            line_start = cur_offset;
        }
    }
    if (cur_offset > line_start) {
        popLine(&lines, cur_offset - line_start, line_start);
    }

    printf("======    TABLE   ======\n");
    printf("number size offset\n");
    for (int i = 0; i < lines.len; ++i) {
        printf("  %d     %d      %d\n", i + 1, lines.list[i].len_line, lines.list[i].offset);
    }
    printf("========================\n");

    if (lines.len == 0) {
        printf("The file is empty\n");
        close(f);
        free(lines.list);
        return 0;
    }

    printf("enter the line number to print (5 seconds):\n");
    fflush(stdout);

    int idx;
    if (!readNumberWithTimeout(&idx)) {
        printf("\nTime is up! Printing the whole file:\n");
        printAll(f);
        close(f);
        free(lines.list);
        return 0;
    }

    while (idx != 0) {
        if (idx < 1 || idx > lines.len) {
            printf("invalid line number\n");
        } else {
            lseek(f, (off_t)lines.list[idx - 1].offset, SEEK_SET);

            char *buf = (char *)malloc(lines.list[idx - 1].len_line + 1);
            if (!buf) {
                perror("malloc");
                break;
            }
            ssize_t n = read(f, buf, lines.list[idx - 1].len_line);
            if (n < 0) {
                perror("read");
                free(buf);
                break;
            }
            buf[n] = '\0';
            if (n > 0 && buf[n - 1] == '\n') {
                buf[n - 1] = '\0';
            }
            printf("%s\n", buf);
            free(buf);
        }

        printf("enter the line number to print\n");
        while (!readNumber(&idx)) {
            printf("enter the line number to print\n");
        }
    }

    close(f);
    free(lines.list);
    return 0;
}
