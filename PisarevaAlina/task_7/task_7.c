#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef struct { int len_line, offset; } Line;
typedef struct { int len, v; Line *list; } Array;

static sigjmp_buf timeout_jmp;

void popLine(Array *lines, int len_line, int offset) {
    if (lines->len == lines->v) {
        lines->v *= 2;
        lines->list = realloc(lines->list, sizeof(Line) * lines->v);
    }
    lines->list[lines->len].len_line = len_line;
    lines->list[lines->len++].offset = offset;
}

void on_alarm(int sig) { (void)sig; siglongjmp(timeout_jmp, 1); }

int readNumber(int *idx) {
    if (scanf("%d", idx) == 1) return 1;
    printf("invalid input\n");
    while (getchar() != '\n' && !feof(stdin));
    return 0;
}

int readNumberWithTimeout(int *idx) {
    if (sigsetjmp(timeout_jmp, 1)) return 0;
    struct sigaction sa = { .sa_handler = on_alarm };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    alarm(5);
    int ok = readNumber(idx);
    alarm(0);
    return ok;
}

int main(int argc, char **argv) {
    if (argc == 1) { printf("The file name has not been passed\n"); return 1; }

    Array lines = { 0, 20, malloc(sizeof(Line) * 20) };
    int f = open(argv[1], O_RDONLY);
    if (f == -1) { perror("open"); free(lines.list); return 1; }

    struct stat st;
    if (fstat(f, &st) == -1) { perror("fstat"); close(f); free(lines.list); return 1; }
    size_t size = st.st_size;

    char *data = size ? mmap(NULL, size, PROT_READ, MAP_PRIVATE, f, 0) : NULL;
    if (size && data == MAP_FAILED) { perror("mmap"); close(f); free(lines.list); return 1; }

    int start = 0;
    for (size_t i = 0; i < size; ++i)
        if (data[i] == '\n') { popLine(&lines, i - start + 1, start); start = i + 1; }
    if ((size_t)start < size) popLine(&lines, size - start, start);

    printf("======    TABLE   ======\nnumber size offset\n");
    for (int i = 0; i < lines.len; ++i)
        printf("  %d     %d      %d\n", i + 1, lines.list[i].len_line, lines.list[i].offset);
    printf("========================\n");

    if (lines.len == 0) {
        printf("The file is empty\n");
        if (data) munmap(data, size);
        close(f); free(lines.list); return 0;
    }

    printf("enter the line number to print (5 seconds):\n");
    fflush(stdout);

    int idx;
    if (!readNumberWithTimeout(&idx)) {
        printf("\nTime is up! Printing the whole file:\n");
        if (size) write(STDOUT_FILENO, data, size);
        if (data) munmap(data, size);
        close(f); free(lines.list); return 0;
    }

    while (idx != 0) {
        if (idx < 1 || idx > lines.len) {
            printf("invalid line number\n");
        } else {
            Line *ln = &lines.list[idx - 1];
            ssize_t n = ln->len_line;
            if (data[ln->offset + n - 1] == '\n') n--;
            if (write(STDOUT_FILENO, data + ln->offset, n) != n) perror("write");
            if (write(STDOUT_FILENO, "\n", 1) != 1) perror("write");
        }
        printf("enter the line number to print\n");
        while (!readNumber(&idx)) printf("enter the line number to print\n");
    }

    if (data) munmap(data, size);
    close(f); free(lines.list);
    return 0;
}
