#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct LineInfo {
    off_t offset;
    size_t length;
};

char *data;
size_t file_size;

volatile sig_atomic_t timed_out = 0;

void timeout(int signo)
{
    (void)signo;

    timed_out = 1;
}

int main(int argc, char *argv[])
{
    int fd;
    struct stat st;

    struct LineInfo lines[1000];

    int line_count = 0;
    off_t line_start = 0;
    off_t current;

    int i;

    long number;
    char input[100];
    char *input_end;

    struct LineInfo info;

    struct sigaction sa;

    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    /*
        Получаем размер файла.
    */
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    file_size = (size_t)st.st_size;

    /*
        Отображаем файл в память.
    */
    if (file_size > 0) {

        data = mmap(
            NULL,
            file_size,
            PROT_READ,
            MAP_PRIVATE,
            fd,
            0
        );

        if (data == MAP_FAILED) {
            perror("mmap");
            close(fd);
            return 1;
        }

    } else {
        data = NULL;
    }

    /*
        Настраиваем SIGALRM.
    */
    sa.sa_handler = timeout;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");

        if (file_size > 0) {
            munmap(data, file_size);
        }

        close(fd);
        return 1;
    }

    /*
        Строим таблицу строк.

        Здесь уже нет read() и lseek().
        Просто смотрим содержимое data[].
    */
    for (current = 0;
         current < (off_t)file_size;
         current++) {

        if (data[current] == '\n') {

            lines[line_count].offset =
                line_start;

            lines[line_count].length =
                current - line_start + 1;

            line_count++;

            line_start = current + 1;
        }
    }

    /*
        Последняя строка без '\n'.
    */
    if ((off_t)file_size > line_start) {

        lines[line_count].offset =
            line_start;

        lines[line_count].length =
            file_size - line_start;

        line_count++;
    }

    /*
        Таблица строк.
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
        Один alarm на проверку пользователя.
    */
    alarm(5);

    while (1) {

        printf("Enter line number (0 to exit): ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {

            /*
                Если fgets прервал именно SIGALRM,
                значит пользователь ничего не ввёл.
            */
            if (timed_out) {

                printf("\n");

                if (file_size > 0) {
                    fwrite(
                        data,
                        1,
                        file_size,
                        stdout
                    );
                }
            }

            break;
        }

        /*
            Пользователь что-то ввёл.

            Всё — alarm больше не нужен.
        */
        alarm(0);

        number = strtol(input, &input_end, 10);

        if (input_end == input ||
            (*input_end != '\n' &&
             *input_end != '\0')) {

            printf("Please enter a number\n");
            continue;
        }

        if (number == 0) {
            break;
        }

        if (number < 1 ||
            number > line_count) {

            printf("Invalid line number\n");
            continue;
        }

        info = lines[number - 1];

        /*
            В task6 здесь были lseek + read.

            Здесь файл уже находится в памяти.
        */
        fwrite(
            data + info.offset,
            1,
            info.length,
            stdout
        );
    }

    alarm(0);

    if (file_size > 0) {
        munmap(data, file_size);
    }

    close(fd);

    return 0;
}
