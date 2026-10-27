#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int c, char **v) {
    int fd = open(v[1], O_RDONLY), n = 0;
    long off[1000]; int len[1000];
    char ch, buf[4096];
    long pos = 0, st = 0;

    while (read(fd, &ch, 1) == 1){
        if (ch == '\n'){ 
            off[n]=st; len[n]=pos-st; n++; st=pos+1; 
        }
        pos++;
    }
    if (pos > st){ 
        off[n]=st; len[n]=pos-st; n++; 
    }

    for (int i = 0; i < n; i++){
        printf("%d: off=%ld len=%d\n", i, off[i], len[i]);
    }

    int k;
    while (printf("Номер (0-выход): "), scanf("%d",&k)==1 && k) {
        if (k<0 || k>=n){
            puts("нет строки"); continue; 
        }
        lseek(fd, off[k], SEEK_SET);
        int l = read(fd, buf, len[k]);
        buf[l] = 0;
        puts(buf);
    }
    close(fd);
}
