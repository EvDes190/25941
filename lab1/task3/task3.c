#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
void print_ids(void)
{
    printf("Real UID:      %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
}
void try_open(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return; 
    }
    printf("File opened successfully\n");
    fclose(file);
}
int main(int argc, char *argv[])
{
    if (argc != 2) { 
        fprintf(stderr, "Usage: %s filename\n", argv[0]); 
        return 1; 
    }
    printf("=== BEFORE setuid ===\n");
    print_ids();
    try_open(argv[1]);
    printf("\n=== setuid ===\n");
    /* Делаем эффективный UID равным реальному. */
    if (setuid(getuid()) == -1) { 
        perror("setuid"); 
        return 1; 
    }
    printf("\n=== AFTER setuid ===\n");
    print_ids();
    try_open(argv[1]);
    return 0;
}

