#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
    FILE *fp;

    printf("real uid: %d, effective uid: %d\n", getuid(), geteuid());

    fp = fopen("data.txt", "r");
    if (fp != NULL) {
        fclose(fp);
    } else {
        perror("fopen");
    }

    setuid(getuid());

    printf("real uid: %d, effective uid: %d\n", getuid(), geteuid());

    fp = fopen("data.txt", "r");
    if (fp != NULL) {
        fclose(fp);
    } else {
        perror("fopen");
    }

    return 0;
}
