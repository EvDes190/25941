#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
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
    off_t begin = 0, cur_pos;
    char c, text[1024];

    int file = open("file.txt", O_RDONLY);
    if (file == -1) {
        perror("open");
        return 1;
    }

    /* строим таблицу: на каждом '\n' записываем начало и длину строки */
    while (total < 1000 && read(file, &c, 1) == 1) {
        if (c == '\n') {
            cur_pos = lseek(file, 0L, SEEK_CUR);
            line_start[total] = begin;
            line_len[total] = cur_pos - begin - 1;
            total++;
            begin = cur_pos;
        }
    }

    /* последняя строка без '\n' в конце файла */
    cur_pos = lseek(file, 0L, SEEK_CUR);
    if (cur_pos > begin && total < 1000) {
        line_start[total] = begin;
        line_len[total] = cur_pos - begin;
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
            lseek(file, 0L, SEEK_SET);
            ssize_t got;
            while ((got = read(file, text, sizeof text)) > 0)
                fwrite(text, 1, got, stdout);
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
        lseek(file, line_start[num - 1], SEEK_SET);
        read(file, text, line_len[num - 1]);
        text[line_len[num - 1]] = '\0';
        printf("%s\n", text);
    }

    close(file);
    return 0;
}