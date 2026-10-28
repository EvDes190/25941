#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int main(void)
{
    FILE *f;

    printf("\n  Before euid change \n");
    printf("uid = %d, euid = %d\n", getuid(), geteuid());

    f = fopen("file", "r");
    if (f == NULL) {
        perror("fopen");
    } else {
        printf("file opened\n");
        fclose(f);
    }

    if (setuid(getuid()) == -1) {
        perror("setuid");
    }

    printf("\n After euid change \n");
    printf("uid = %d, euid = %d\n", getuid(), geteuid());

    f = fopen("file", "r");
    if (f == NULL) {
        perror("fopen");
    } else {
        printf("file opened\n");
        fclose(f);
    }

    return 0;
}
