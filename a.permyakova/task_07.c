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

int main() {
    off_t line_start[1000];
    int line_len[1000];
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
    if (size == 0) {
        close(file);
        return 0;
    }

    
    char *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, file, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

  
    for (off_t i = 0; i < size && total < 1000; i++) {
        if (map[i] == '\n') {
            line_start[total] = begin;
            line_len[total] = i - begin;
            total++;
            begin = i + 1;
        }
    }

    if (begin < size && total < 1000) {
        line_start[total] = begin;
        line_len[total] = size - begin;
        total++;
    }

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

        if (expired) {
            printf("\n");
            fwrite(map, 1, size, stdout);
            break;
        }
        if (res == EOF || (res == 1 && num == 0))
            break;
        if (res != 1){
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
