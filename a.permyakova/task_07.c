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

int main(void) {
    int fd = open("file.txt", O_RDONLY);
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

    size_t size = st.st_size;
    char *data = NULL;

    if (size > 0) {
        data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED) {
            perror("mmap");
            close(fd);
            return 1;
        }
    }

    // таблица 
    size_t *offset = NULL, *length = NULL;
    size_t count = 0, cap = 0, start = 0;

    for (size_t i = 0; i <= size; i++) {
        int end_of_line = (i < size && data[i] == '\n');
        int end_of_file = (i == size && start < size);

        if (!end_of_line && !end_of_file)
            continue;

        if (count == cap) {
            cap = cap ? cap * 2 : 16;
            size_t *o = realloc(offset, cap * sizeof(size_t));
            size_t *l = realloc(length, cap * sizeof(size_t));
            if (o) offset = o;
            if (l) length = l;
            if (o == NULL || l == NULL) {
                perror("realloc");
                return 1;
            }
        }
        offset[count] = start;
        length[count++] = i - start;
        start = i + 1;
    }

    printf("Line table:\n");
    for (size_t j = 0; j < count; j++) {
        printf("line %zu: offset=%zu length=%zu\n", j + 1, offset[j], length[j]);
    }

    struct sigaction act = {0};
    act.sa_handler = on_timer;
    sigaction(SIGALRM, &act, NULL);

    char *input = NULL;
    size_t input_cap = 0;

    while (1) {
        printf("Enter line number (5 sec, 0 - exit): ");
        fflush(stdout);

        expired = 0;
        alarm(5);
        ssize_t res = getline(&input, &input_cap, stdin);
        alarm(0);

        if (expired) {
            printf("\n");
            if (size > 0)
                fwrite(data, 1, size, stdout);
            break;
        }
        if (res == -1)
            break;

        sanitize(input);

        char *p = input;
        while (isspace((unsigned char)*p)) p++;

        char *end;
        long n = strtol(p, &end, 10);
        while (isspace((unsigned char)*end)) end++;

        if (p == end || *end || n < 0) {
            printf("Enter a number\n");
            continue;
        }
        if (n == 0)
            break;
        if ((size_t)n > count) {
            printf("Not this line\n");
            continue;
        }

        printf("%.*s\n", (int)length[n - 1], data + offset[n - 1]);
    }

    free(input);
    free(offset);
    free(length);
    if (size > 0)
        munmap(data, size);
    close(fd);
    return 0;
}
