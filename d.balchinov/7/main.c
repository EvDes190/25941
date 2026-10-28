#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define B 100

int descript;
char filename[PATH_MAX] = "poem";

typedef struct line_t {
    int size;
    int shift;
    char *data;
} line_t;

typedef struct lines_t {
    line_t *lines_data;
    int size;
} lines_t;

void handler() {
    char buf[B + 1];
    int size;

    lseek(descript, 0, SEEK_SET);
    fputc('\n', stdout);
    while((size = read(descript, buf, B)) > 0) {
        buf[size] = '\0';
        printf("%s", buf);
    }

    exit(0);
}

lines_t * init_lines() {
    char buf[B + 1];
    lines_t *lines;
    int read_size;
    int line_num;
    int lines_count;

    lseek(descript, 0, SEEK_SET);
    lines_count = 1;
    while((read_size = read(descript, buf, B)) > 0) {
        for (int i = 0; i < read_size; i++)
            lines_count += buf[i] == '\n';
    }
    lines = malloc(sizeof(lines_t));
    lines->size = lines_count;
    lines->lines_data = malloc(lines_count * sizeof(line_t));

    lseek(descript, 0, SEEK_SET);
    lines->lines_data[0].shift = 0;
    lines->lines_data[0].size = 0;

    line_num = 0;
    int shift = 0;
    while((read_size = read(descript, buf, B)) > 0) {
        for (int i = 0; i < read_size; i++) {
            if ((buf[i] == '\n') || (((buf[i] != '\n' && (i == read_size -1)) && (line_num == lines->size - 1)))) {
                lines->lines_data[line_num].size = shift - lines->lines_data[line_num].shift;
                lines->lines_data[line_num].data = malloc(lines->lines_data[line_num].size + 1);
                lines->lines_data[line_num].data[lines->lines_data[line_num].size] = '\0';
                line_num++;
                if (line_num < lines->size) {
                    lines->lines_data[line_num].shift = shift + 1;
                    lines->lines_data[line_num].size = 0;
                }
            }
            shift++;
        }
    }

    if (lines->lines_data[line_num].size == 0)
        lines->lines_data[line_num].data = calloc(1, sizeof(char));

    lseek(descript, 0, SEEK_SET);
    line_num = 0;
    int line_len = 0;
    while((read_size = read(descript, buf, B)) > 0) {
        for (int i = 0; i < read_size; i++) {
            if (buf[i] == '\n') {
                line_len = -1;
                line_num++;
            } else
                lines->lines_data[line_num].data[line_len] = buf[i];
            line_len++;
        }
    }
    return lines;
}

void print_table(lines_t *lines) {
    printf("     shift       size\n");
    for (int i = 0; i < lines->size; i++)
        printf("%10d %10d\n", lines->lines_data[i].shift, lines->lines_data[i].size);
}

void print_line(lines_t *lines, int num) {
    puts(lines->lines_data[num].data);
    return;
}

int main(int argc, char *argv[]) {
    lines_t *lines;
    
    if (argc > 1) {
        memcpy(filename, argv[1], strlen(argv[1]));
        filename[strlen(argv[1])] = '\0';
    }
    descript = open(filename, O_RDONLY);
    lines = init_lines();
    signal(SIGALRM, handler);
    printf("> ");
    alarm(5);
    int scan_res;
    int buf;
    scan_res = scanf("%d", &buf);
    alarm(0);
    while ((buf != 0 && (scan_res == 1))) {
        print_line(lines, buf - 1);
        printf("> ");
        scan_res = scanf("%d", &buf);
    }

    if (buf == 0)
        print_table(lines);

    if (scan_res == 0)
        puts("Not A Number!");
    return 0;
}