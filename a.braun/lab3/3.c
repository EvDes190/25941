#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("RUID = %d, EUID = %d\n", getuid(), geteuid());
    FILE *f = fopen("input.txt", "r");
    if (!f) perror("fopen");
    else fclose(f);
    if (setuid(getuid()) == -1) perror("setuid");
    printf("RUID = %d, EUID = %d\n", getuid(), geteuid());
    FILE *fс = fopen("input.txt", "r");
    if (!fс) perror("fopen");
    else fclose(fс);
    return 0;
}