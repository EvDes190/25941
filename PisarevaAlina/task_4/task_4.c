#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct node {
    char *str;
    struct node *next;
};

static size_t sanitize(char *s)
{
    size_t j = 0;
    for (size_t i = 0; s[i] != '\0'; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '\n' || c == '\t' || (c >= 32 && c != 127)) {
            s[j++] = s[i];
        }
    }
    s[j] = '\0';
    return j;
}

int main(void)
{
    char buf[256];
    struct node *head = NULL;
    struct node *tail = NULL;

    while (fgets(buf, sizeof(buf), stdin) != NULL) {

        size_t len = strlen(buf);
        if (len == sizeof(buf) - 1 && buf[len - 1] != '\n') {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }
        }

        if (buf[0] == '.')
            break;

        len = sanitize(buf);

        struct node *p = malloc(sizeof(struct node));
        if (p == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        p->str = malloc(len + 1);
        if (p->str == NULL) {
            perror("malloc");
            free(p);
            exit(EXIT_FAILURE);
        }

        memcpy(p->str, buf, len + 1);
        p->next = NULL;

        if (head == NULL)
            head = p;
        else
            tail->next = p;

        tail = p;
    }

    struct node *p = head;
    while (p != NULL) {
        printf("%s", p->str);
        p = p->next;
    }

    p = head;
    while (p != NULL) {
        struct node *next = p->next;
        free(p->str);
        free(p);
        p = next;
    }

    return 0;
}
