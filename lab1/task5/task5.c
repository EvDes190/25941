#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

struct LineInfo {
    off_t offset;
    size_t length;
};

int main(int argc, char *argv[]) {
    int fd;
    struct LineInfo lines[1000];
    int line_count = 0;
    off_t line_start = 0;
    off_t current;
    off_t file_end;
    char c;
    int i;
    long number;
    char input[100];
    char *input_end;
    char buffer[1024];
    struct LineInfo info;
    ssize_t bytes_read;

    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    while (read(fd, &c, 1) == 1) {
        if (c == '\n') {
            current = lseek(fd, 0L, SEEK_CUR);

            lines[line_count].offset = line_start;
            lines[line_count].length = current - line_start;

            line_count++;
            line_start = current;
        }
    }

    file_end = lseek(fd, 0L, SEEK_CUR);

    if (file_end > line_start) {
        lines[line_count].offset = line_start;
        lines[line_count].length = file_end - line_start;
        line_count++;
    }

    printf("Line table:\n");

    for (i = 0; i < line_count; i++) {
        printf("Line %d: offset=%ld length=%lu\n",
               i + 1,
               (long)lines[i].offset,
               (unsigned long)lines[i].length);
    }

    while (1) {
        printf("Enter line number (0 to exit): ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        number = strtol(input, &input_end, 10);

        if (input_end == input ||
            (*input_end != '\n' && *input_end != '\0')) {
            printf("Please enter a number\n");
            continue;
        }

        if (number == 0) {
            break;
        }

        if (number < 1 || number > line_count) {
            printf("Invalid line number\n");
            continue;
        }

        info = lines[number - 1];

        lseek(fd, info.offset, SEEK_SET);

        if (info.length >= sizeof(buffer)) {
            printf("Line is too long\n");
            continue;
        }

        bytes_read = read(fd, buffer, info.length);

        if (bytes_read == -1) {
            perror("read");
            break;
        }

        buffer[bytes_read] = '\0';

        printf("%s", buffer);
    }

    close(fd);

    return 0;
}
