#include <stdio.h>
#include <unistd.h>

void print_ids(void) {
    printf("uid = %d, euid = %d\n", getuid(), geteuid());
}

void try_open(const char *name) {
    FILE *f = fopen(name, "r");

    if (f == NULL) {
        perror("fopen");
        return;
    }
    printf("file opened\n");
    fclose(f);
}

int main(int argc, char *argv[]) {
    const char *name = "file"; 

    if (argc > 1)
        name = argv[1];


    print_ids();
    try_open(name);

    if (setuid(getuid()) == -1)
        perror("setuid");

    print_ids();
    try_open(name);

    return 0;
}