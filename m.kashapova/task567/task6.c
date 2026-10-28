#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

int fd, n;
long off[999]; int len[999];

void bye(int s) {
    char b[4096]; int r;
    lseek(fd, 0, 0);
    while ((r = read(fd, b, 4096)) > 0) write(1, b, r);
    _exit(0);
}

int main(int c, char **v) {
    fd = open(v[1], O_RDONLY);
    char ch, b[4096]; long p = 0, st = 0;

    while (read(fd, &ch, 1) == 1) {
        if (ch == '\n') { off[n]=st; len[n]=p-st; n++; st=p+1; }
        p++;
    }
    if (p > st) { off[n]=st; len[n]=p-st; n++; }

    signal(SIGALRM, bye);

    int k;
    int first_input_done = 0;

    while (1) {
        printf("Номер (0-выход, 5 сек): "); fflush(stdout);

        if (!first_input_done)
            alarm(5);

        int r = scanf("%d", &k);

        if (!first_input_done)
            alarm(0);

        if (r != 1) { while (getchar() != '\n'); continue; }

        first_input_done = 1;

        if (!k) break;
        if (k < 0 || k >= n) { puts("нет строки"); continue; }
        lseek(fd, off[k], 0);
        int l = read(fd, b, len[k]);
        b[l] = 0;
        puts(b);
    }
    close(fd);
}
