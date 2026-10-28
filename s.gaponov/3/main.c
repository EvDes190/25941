#include <stdio.h>
#include <unistd.h>

void print_ids(void) {
    printf("Real UID: %ld\n", (long)getuid());
    printf("Effective UID: %ld\n", (long)geteuid());
}

void open_file(void) {
    FILE *file = fopen("data.txt", "r");

    if (file == NULL) {
        perror("fopen");
        return;
    }

    printf("File opened successfully\n");
    fclose(file);
}

int main(void) {
    print_ids();
    open_file();

    if (setuid(getuid()) == -1) {
        perror("setuid");
        return 1;
    }

    print_ids();
    open_file();

    return 0;
}