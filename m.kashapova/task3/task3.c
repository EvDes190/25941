#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    const char *filename = "data.txt";
    printf("Before setuid:\n");
    printf("  real UID = %d\n", getuid());
    printf("  effective UID = %d\n", geteuid());

   
    FILE *f = fopen(filename, "r+");
    if (f == NULL) {
        perror("fopen");
    } else {
        printf("File opened OK\n");
        fclose(f);
    }

  
    if (setuid(geteuid()) == -1) {
        perror("setuid");
    }

   
    printf("\nAfter setuid:\n");
    printf("  real UID = %d\n", getuid());
    printf("  effective UID = %d\n", geteuid());

    f = fopen(filename, "r+");
    if (f == NULL) {
        perror("fopen");
    } else {
        printf("File opened OK\n");
        fclose(f);
    }

    return 0;
}
