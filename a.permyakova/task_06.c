#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    
    if (fd == -1) {
        perror("open");
        return 1;
    }

    int count = 0;
    char ch;
    off_t size = 0;
    while (read(fd, &ch, 1) == 1) {
        size++;
        if (ch == '\n')
            count++;
    }
    if (size > 0 && ch != '\n')
        count++;

    off_t *offset = malloc(count * sizeof(off_t) + 1);
    int *length = malloc(count * sizeof(int) + 1);
    if (offset == NULL || length == NULL) {
        perror("malloc");
        return 1;
    }

    lseek(fd, 0L, SEEK_SET);
    off_t start = 0, pos;
    int i = 0;
    while (read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            pos = lseek(fd, 0L, SEEK_CUR);
            offset[i] = start;
            length[i] = pos - start - 1;
            i++;
            start = pos;
        }
    }

    if (i < count) {
        offset[i] = start;
        length[i] = size - start;
    }

    printf("Line table:\n");
    for (int j = 0; j < count; j++) {
        printf("line %d: offset=%ld length=%d\n", j + 1, (long)offset[j], length[j]);
    }

    char *input = NULL;
    size_t cap = 0;

    struct sigaction act = {0};
    act.sa_handler = on_timer;
    sigaction(SIGALRM, &act, NULL);

    printf("Enter line number (0 - exit): ");
    fflush(stdout);

    expired = 0;
    alarm(5);
    ssize_t res = getline(&input, &cap, stdin);
    alarm(0);

    if (expired) {
        printf("\n");
        lseek(fd, 0L, SEEK_SET);
        while (read(fd, &ch, 1) == 1)
            putchar(ch);
        free(input);
        free(offset);
        free(length);
        close(fd);
        return 0;
    }

    if (res != -1) {
        sanitize(input);

        int n = atoi(input);
        if (n == 0 && input[0] != '0') {
            printf("Enter a number\n");
        } else if (n == 0) {
            free(input);
            free(offset);
            free(length);
            close(fd);
            return 0;
        } else if (n < 1 || n > count) {
            printf("Not this line\n");
        } else {
            char *buf = malloc(length[n - 1] + 1);
            lseek(fd, offset[n - 1], SEEK_SET);
            ssize_t got = read(fd, buf, length[n - 1]);
            if (got < 0)
                got = 0;
            buf[got] = '\0';
            printf("%s\n", buf);
            free(buf);
        }
    }

    while (1) {
        printf("Enter line number (0 - exit): ");
        fflush(stdout);

        if (getline(&input, &cap, stdin) == -1)
            break;
        sanitize(input);

        int n = atoi(input);
        if (n == 0 && input[0] != '0') {
            printf("Enter a number\n");
            continue;
        }
        if (n == 0)
            break;
        if (n < 1 || n > count) {
            printf("Not this line\n");
            continue;
        }

        char *buf = malloc(length[n - 1] + 1);
        lseek(fd, offset[n - 1], SEEK_SET);
        ssize_t got = read(fd, buf, length[n - 1]);
        if (got < 0)
            got = 0;
        buf[got] = '\0';
        printf("%s\n", buf);
        free(buf);
    }

    free(input);
    free(offset);
    free(length);
    close(fd);
    return 0;
}
