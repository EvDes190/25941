#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    off_t offset[1000];
    int length[1000];
    int count = 0, n;
    off_t start = 0, pos;
    char ch, buf[1024];

    int fd = open("file.txt", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }


    while (count < 1000 && read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            pos = lseek(fd, 0L, SEEK_CUR);
            offset[count] = start;
            length[count] = pos - start - 1;
            count++;
            start = pos;
        }
    }


    while (1) {
        printf("Enter line number (0 - exit): ");
        if (scanf("%d", &n) != 1 || n == 0)
            break;
        if (n < 1 || n > count) {
            printf("Not this line\n");
            continue;
        }
        lseek(fd, offset[n - 1], SEEK_SET);
        read(fd, buf, length[n - 1]);
        buf[length[n - 1]] = '\0';
        printf("%s\n", buf);
    }

    close(fd);
    return 0;
}