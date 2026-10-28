#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

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

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t size = (size_t)st.st_size;
    char *data = NULL;

    if (size > 0) {
        data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED) {
            perror("mmap");
            close(fd);
            return 1;
        }
    }

    int count = 0;
    size_t i;
    for (i = 0; i < size; i++) {
        if (data[i] == '\n')
            count++;
    }
    if (size > 0 && data[size - 1] != '\n')
        count++;

    off_t *offset = malloc(count * sizeof(off_t) + 1);
    int *length = malloc(count * sizeof(int) + 1);
    if (offset == NULL || length == NULL) {
        perror("malloc");
        if (size > 0)
            munmap(data, size);
        close(fd);
        return 1;
    }

    size_t start = 0;
    int idx = 0;
    for (i = 0; i < size; i++) {
        if (data[i] == '\n') {
            offset[idx] = (off_t)start;
            length[idx] = (int)(i - start);
            idx++;
            start = i + 1;
        }
    }
    if (idx < count) {
        offset[idx] = (off_t)start;
        length[idx] = (int)(size - start);
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
        if (size > 0)
            fwrite(data, 1, size, stdout);
        free(input);
        free(offset);
        free(length);
        if (size > 0)
            munmap(data, size);
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
            if (size > 0)
                munmap(data, size);
            close(fd);
            return 0;
        } else if (n < 1 || n > count) {
            printf("Not this line\n");
        } else {
            printf("%.*s\n", length[n - 1], data + offset[n - 1]);
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

        printf("%.*s\n", length[n - 1], data + offset[n - 1]);
    }

    free(input);
    free(offset);
    free(length);
    if (size > 0)
        munmap(data, size);
    close(fd);
    return 0;
}
