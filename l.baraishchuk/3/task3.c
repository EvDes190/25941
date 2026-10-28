#define _XOPEN_SOURCE 600

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define DATA_FILE "file"

static void print_ids_and_open(const char *stage)
{
    FILE *stream;

    printf("%s\n", stage);
    printf("real uid = %lu, effective uid = %lu\n",
           (unsigned long)getuid(), (unsigned long)geteuid());
    fflush(stdout);

    /* r+ requests both reading and writing of the existing file. */
    stream = fopen(DATA_FILE, "r+");
    if (stream == NULL) {
        perror("fopen");
        return;
    }

    printf("fopen: success\n");

    if (fclose(stream) == EOF)
        perror("fclose");
}

int main(void)
{
    print_ids_and_open("before setuid(getuid())");

    if (setuid(getuid()) == -1) {
        perror("setuid");
        return 1;
    }

    print_ids_and_open("after setuid(getuid())");
    return 0;
}
