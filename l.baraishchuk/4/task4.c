#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
    char *str;
    struct node *next;
};

void clean_input(char *s)
{
    char *src = s;
    char *dst = s;

    while (*src != '\0') {

        if ((unsigned char)*src == 27) {   /* ESC */
            src++;

            if (*src == '[') {
                src++;

                while (*src != '\0') {
                    if (*src >= '@' && *src <= '~') {
                        src++;
                        break;
                    }

                    src++;
                }
            }
        }
        else {
            *dst = *src;
            dst++;
            src++;
        }
    }

    *dst = '\0';
}

int main()
{
    char buf[256];

    struct node *head = NULL;
    struct node *tail = NULL;
    struct node *p;

    while (fgets(buf, sizeof(buf), stdin) != NULL) {

        clean_input(buf);

        if (buf[0] == '.')
            break;

        p = malloc(sizeof(struct node));

        if (p == NULL) {
            perror("malloc");
            return 1;
        }

        p->str = malloc(strlen(buf) + 1);

        if (p->str == NULL) {
            perror("malloc");
            return 1;
        }

        strcpy(p->str, buf);

        p->next = NULL;

        if (head == NULL)
            head = p;
        else
            tail->next = p;

        tail = p;
    }

    p = head;

    while (p != NULL) {
        printf("%s", p->str);
        p = p->next;
    }

    return 0;
}