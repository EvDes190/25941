#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct LineInfo {
    off_t offset;
    size_t length;
};

char *file_data;
size_t file_size;

void timeout(int signal) {
    (void)signal;

    write(STDOUT_FILENO, "\n", 1);

    size_t written = 0;

    while (written < file_size) {
        ssize_t n =
            write(STDOUT_FILENO, file_data + written, file_size - written);

        if (n <= 0) {
            break;
        }

        written += n;
    }

    _exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
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

    file_size = st.st_size;

    if (file_size == 0) {
        printf("File is empty\n");
        close(fd);
        return 0;
    }

    file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (file_data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    signal(SIGALRM, timeout);

    struct LineInfo lines[1000];
    int line_count = 0;

    size_t line_start = 0;

    for (size_t i = 0; i < file_size; i++) {
        if (file_data[i] == '\n') {
            if (line_count >= 1000) {
                printf("Too many lines\n");
                munmap(file_data, file_size);
                close(fd);
                return 1;
            }

            lines[line_count].offset = line_start;
            lines[line_count].length = i - line_start + 1;

            line_count++;
            line_start = i + 1;
        }
    }

    if (line_start < file_size) {
        if (line_count >= 1000) {
            printf("Too many lines\n");
            munmap(file_data, file_size);
            close(fd);
            return 1;
        }

        lines[line_count].offset = line_start;
        lines[line_count].length = file_size - line_start;
        line_count++;
    }

    printf("Line table:\n");

    for (int i = 0; i < line_count; i++) {
        printf("Line %d: offset=%ld length=%zu\n", i + 1, (long)lines[i].offset,
               lines[i].length);
    }

    long number;
    char input[100];
    char *input_end;

    alarm(5);

    while (1) {
        printf("Enter line number (0 to exit): ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

	alarm(0);

        number = strtol(input, &input_end, 10);

        if (input_end == input || (*input_end != '\n' && *input_end != '\0')) {
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

        struct LineInfo info = lines[number - 1];

        printf("%.*s", (int)info.length, file_data + info.offset);
    }

    alarm(0);
    munmap(file_data, file_size);
    close(fd);

    return 0;
}
