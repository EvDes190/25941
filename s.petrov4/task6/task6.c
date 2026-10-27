#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

struct LineInfo {
    off_t offset;
    size_t length;
};

int fd;

void timeout(int signo)
{
    char buffer[1024];
    ssize_t bytes_read;

    (void)signo;

    write(STDOUT_FILENO, "\n", 1);

    lseek(fd, 0L, SEEK_SET);

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }

    _exit(0);
}

int main(int argc, char *argv[])
{
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

    signal(SIGALRM, timeout);

    /*
        Строим таблицу строк.
    */
    while (read(fd, &c, 1) == 1) {

        if (c == '\n') {

            current = lseek(fd, 0L, SEEK_CUR);

            lines[line_count].offset = line_start;
            lines[line_count].length =
                current - line_start;

            line_count++;
            line_start = current;
        }
    }

    /*
        Последняя строка без '\n'.
    */
    file_end = lseek(fd, 0L, SEEK_CUR);

    if (file_end > line_start) {

        lines[line_count].offset = line_start;
        lines[line_count].length =
            file_end - line_start;

        line_count++;
    }

    /*
        Печатаем таблицу.
    */
    printf("Line table:\n");

    for (i = 0; i < line_count; i++) {

        printf(
            "Line %d: offset=%ld length=%lu\n",
            i + 1,
            (long)lines[i].offset,
            (unsigned long)lines[i].length
        );
    }

    /*
        Проверяем, есть ли пользователь.

        Если за 5 секунд вообще ничего
        не ввёл — timeout().
    */
    alarm(5);

    while (1) {

        printf("Enter line number (0 to exit): ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        /*
            Пользователь что-то ввёл.

            Неважно что:
            4, 100, abc...

            Значит он здесь.
            Таймер больше не нужен.
        */
        alarm(0);

        number = strtol(input, &input_end, 10);

        /*
            Проверка: вообще число или нет.
        */
        if (input_end == input ||
            (*input_end != '\n' &&
             *input_end != '\0')) {

            printf("Please enter a number\n");
            continue;
        }

        /*
            0 — выход.
        */
        if (number == 0) {
            break;
        }

        /*
            Проверяем существование строки.
        */
        if (number < 1 ||
            number > line_count) {

            printf("Invalid line number\n");
            continue;
        }

        /*
            Дальше обычная задача 5.
        */
        info = lines[number - 1];

        lseek(fd, info.offset, SEEK_SET);

        if (info.length >= sizeof(buffer)) {
            printf("Line is too long\n");
            continue;
        }

        bytes_read =
            read(fd, buffer, info.length);

        if (bytes_read == -1) {
            perror("read");
            break;
        }

        buffer[bytes_read] = '\0';

        printf("%s", buffer);
    }

    alarm(0);
    close(fd);

    return 0;
}
