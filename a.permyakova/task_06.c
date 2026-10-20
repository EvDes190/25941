#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

struct line {
    off_t start; // отступ
    int len; //длина
};

static volatile sig_atomic_t expired = 0;

static void on_timer(int sig) {
    (void)sig;
    expired = 1;
}

static void sanitize(char *s) {
    char *start = s, *w = s;

    while (*s) {
        if (*s == 27) {
            s++;
            if (*s == '[' || *s == 'O') {
                s++;
                while (*s && !isalpha((unsigned char)*s) && *s != '~')
                    s++;
                if (*s)
                    s++;
            }
        } else if (*s == 8 || *s == 127) {
            if (w > start)
                w--;
            s++;
        } else if (iscntrl((unsigned char)*s) && *s != '\n') {
            s++;
        } else {
            *w++ = *s++;
        }
    }
    *w = '\0';
}

int main() {
    struct line *lines = NULL;
    int total = 0;
    off_t begin = 0, pos;
    char c;

    int file = open("file.txt", O_RDONLY);
    if (file == -1) {
        perror("open");
        return 1;
    }

    while (read(file, &c, 1) == 1) {
        if (c == '\n') {
            pos = lseek(file, 0L, SEEK_CUR);
            lines = realloc(lines, (total + 1) * sizeof(struct line));
            lines[total].start = begin;
            lines[total].len = pos - begin - 1;
            total++;
            begin = pos;
        }
    }
    // таблица
    pos = lseek(file, 0L, SEEK_CUR);
    if (pos > begin) {
        lines = realloc(lines, (total + 1) * sizeof(struct line));
        lines[total].start = begin;
        lines[total].len = pos - begin;
        total++;
    }

    struct sigaction act = {0};
    act.sa_handler = on_timer;
    sigaction(SIGALRM, &act, NULL);

    char *input = NULL;
    size_t cap = 0;

    while (1) {
        printf("Enter line number (5 sec, 0 - exit): ");
        fflush(stdout);

        expired = 0;
        alarm(5);
        ssize_t res = getline(&input, &cap, stdin);
        alarm(0);

        if (expired) {
            printf("\n");
            lseek(file, 0L, SEEK_SET);
            while (read(file, &c, 1) == 1)
                putchar(c);
            break;
        }
        if (res == -1)
            break;

        sanitize(input);
        int num = atoi(input);

        if (num == 0 && input[0] != '0') {
            printf("Enter a number\n");
            continue;
        }
        if (num == 0)
            break;
        if (num < 1 || num > total) {
            printf("Not this line\n");
            continue;
        }

        char *text = malloc(lines[num - 1].len + 1);
        lseek(file, lines[num - 1].start, SEEK_SET);
        ssize_t got = read(file, text, lines[num - 1].len);
        if (got < 0)
            got = 0;
        text[got] = '\0';
        printf("%s\n", text);
        free(text);
    }

    free(input);
    free(lines);
    close(file);
    return 0;
}