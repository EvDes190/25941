#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static volatile sig_atomic_t expired = 0;

void on_timer(int sig)
{
    (void)sig;
    expired = 1;
}

int main(void)
{
    off_t line_start[1000];   /* где начинается каждая строка */
    int line_len[1000];       /* длина каждой строки без '\n' */
    int total = 0, num;
    off_t begin = 0;
    struct stat st;

    int file = open("file.txt", O_RDONLY);
    if (file == -1) {
        perror("open");
        return 1;
    }

    if (fstat(file, &st) == -1) {
        perror("fstat");
        return 1;
    }
    off_t size = st.st_size;
    if (size == 0) {          /* пустой файл: mmap с нулевой длиной не работает */
        close(file);
        return 0;
    }

    /* отображаем файл в память: теперь он доступен как массив map[] */
    char *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, file, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    /* строим таблицу: на каждом '\n' записываем начало и длину строки */
    for (off_t i = 0; i < size && total < 1000; i++) {
        if (map[i] == '\n') {
            line_start[total] = begin;
            line_len[total] = i - begin;
            total++;
            begin = i + 1;
        }
    }

    /* последняя строка без '\n' в конце файла */
    if (begin < size && total < 1000) {
        line_start[total] = begin;
        line_len[total] = size - begin;
        total++;
    }

    /* обработчик без SA_RESTART, чтобы alarm прерывал scanf */
    struct sigaction act;
    act.sa_handler = on_timer;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGALRM, &act, NULL);

    while (1) {
        printf("Enter line number (5 sec, 0 - exit): ");
        fflush(stdout);

        expired = 0;
        alarm(5);
        int res = scanf("%d", &num);
        alarm(0);

        if (expired) {                       /* не успел: печатаем весь файл */
            printf("\n");
            fwrite(map, 1, size, stdout);
            break;
        }
        if (res == EOF || (res == 1 && num == 0))
            break;
        if (res != 1) {                      /* ввели не число: сбросить строку */
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            continue;
        }

        if (num < 1 || num > total) {
            printf("Not this line\n");
            continue;
        }
        fwrite(map + line_start[num - 1], 1, line_len[num - 1], stdout);
        printf("\n");
    }

    munmap(map, size);
    close(file);
    return 0;
}