#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

struct Node {
    char *text;
    struct Node *next;
};

int read_line(char *buffer, size_t size) {
    if (!isatty(STDIN_FILENO))
        return fgets(buffer, size, stdin) != NULL;

    struct termios old, mode;
    if (tcgetattr(STDIN_FILENO, &old) == -1)
        return -1;

    mode = old;
    mode.c_lflag &= ~(ICANON | ECHO | ISIG);
    mode.c_cc[VMIN] = 1;
    mode.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &mode) == -1)
        return -1;

    size_t n = 0;
    unsigned char c;
    int result = 0;

    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == 27) {
            unsigned char x;
            if (read(STDIN_FILENO, &x, 1) != 1)
                break;
            if (x == '[' || x == 'O') {
                do {
                    if (read(STDIN_FILENO, &x, 1) != 1)
                        break;
                } while (x < 64 || x > 126);
            }
            continue;
        }

        if (c == 3) {
            result = -1;
            break;
        }

        if (c == 4)
            break;

        if (c == 127 || c == 8) {
            if (n > 0) {
                do {
                    --n;
                } while (n > 0 && ((unsigned char)buffer[n] & 0xC0) == 0x80);
                write(STDOUT_FILENO, "\b \b", 3);
            }
            continue;
        }

        if (c == '\r' || c == '\n') {
            if (n + 1 < size)
                buffer[n++] = '\n';
            write(STDOUT_FILENO, "\n", 1);
            result = 1;
            break;
        }

        if (c >= 32 && n + 2 < size) {
            buffer[n++] = (char)c;
            write(STDOUT_FILENO, &c, 1);
        }
    }

    if (result == 0 && n > 0)
        result = 1;
    buffer[n] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    return result;
}

int main(void) {
    struct Node *head = NULL, *tail = NULL;
    char buffer[1024];
    int result;

    while ((result = read_line(buffer, sizeof(buffer))) > 0) {
        if (buffer[0] == '.')
            break;

        struct Node *node = malloc(sizeof(struct Node));
        if (node == NULL) {
            perror("malloc");
            break;
        }

        node->text = malloc(strlen(buffer) + 1);
        if (node->text == NULL) {
            perror("malloc");
            free(node);
            break;
        }

        strcpy(node->text, buffer);
        node->next = NULL;

        if (head == NULL)
            head = node;
        else
            tail->next = node;

        tail = node;
    }

    for (struct Node *p = head; p != NULL; p = p->next)
        printf("%s", p->text);

    while (head != NULL) {
        struct Node *next = head->next;
        free(head->text);
        free(head);
        head = next;
    }

    return result < 0 ? 1 : 0;
}