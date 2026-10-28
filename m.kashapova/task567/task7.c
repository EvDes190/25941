#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    int fd = open(argv[1], O_RDONLY);

    struct stat st;
    fstat(fd, &st);

    char *data = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    long off[999];
    int len[999];
    int n = 0;
    long start = 0;

    for (long i = 0; i < st.st_size; i++) {
        if (data[i] == '\n') {
            off[n] = start;
            len[n] = i - start;
            n++;
            start = i + 1;
        }
    }
    if (start < st.st_size) {
        off[n] = start;
        len[n] = st.st_size - start;
        n++;
    }
    int k;
    while (printf("Номер (0-выход): "), scanf("%d", &k) == 1 && k) {
        if (k < 0 || k >= n) {
            puts("нет строки");
            continue;
        }
        write(1, data + off[k], len[k]);
        write(1, "\n", 1);
    }

    munmap(data, st.st_size);
    close(fd);
}
