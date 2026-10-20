#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

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
    int fd = open("file.txt", O_RDONLY);
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


    //таблица
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

    char *input = NULL;
    size_t cap = 0;

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