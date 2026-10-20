#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct node {
    char *text;
    struct node *next;
};

static void sanitize(char *s) {
    char *start = s, *w = s;

    while (*s) {
        if (*s == 27) {
            s++;
            if (*s == '[' || *s == 'O') {
                s++;
                while (*s && !isalpha((unsigned char)*s) && *s != '~')
                    s++;
                if (*s)
                    s++;
            }
        } else if (*s == 8 || *s == 127) {
            if (w > start)
                w--;
            s++;
        } else if (iscntrl((unsigned char)*s) && *s != '\n' && *s != '\t') {
            s++;
        } else {
            *w++ = *s++;
        }
    }
    *w = '\0';
}

int main(void) {
    char *line = NULL;
    size_t cap = 0;
    struct node *first = NULL, *last = NULL, *cur;

    while (getline(&line, &cap, stdin) != -1) {
        sanitize(line);
        if (strcmp(line, ".\n") == 0 || strcmp(line, ".") == 0)
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
    free(line);
    return 0;
}