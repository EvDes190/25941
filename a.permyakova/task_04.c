#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
    char *text;
    struct node *next;
};

int main(void) {
    char line[1024];
    struct node *first = NULL;
    struct node *last = NULL;
    struct node *cur;


    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (line[0] == '.')
            break;

        cur = malloc(sizeof(struct node));
        if (cur == NULL) {
            perror("malloc");
            break;
        }

        cur->text = malloc(strlen(line) + 1);
        if (cur->text == NULL) {
            perror("malloc");
            free(cur);
            break;
        }
        strcpy(cur->text, line);
        cur->next = NULL;

        if (first == NULL)
            first = cur;
        else
            last->next = cur;
        last = cur;
    }

    for (cur = first; cur != NULL; cur = cur->next)
        printf("%s", cur->text);

    while (first != NULL) {
        cur = first;
        first = first->next;
        free(cur->text);
        free(cur);
    }

    return 0;
}