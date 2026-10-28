#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct node_t {
    struct node_t *next;
    char *data;
} node_t;

typedef struct {
    node_t *begin;
    node_t *end;
    int list_size;
} list_t;

void add_node(list_t *list, char *str) {

    node_t *node = malloc(sizeof(node_t));

    node->next = NULL;
    
    node->data = malloc(strlen(str));
    memcpy(node->data, str, strlen(str) - 1);
    node->data[strlen(str) - 1] = '\0';
    if (list->begin == 0) {
        list->begin = node;
        list->end = node;
    } else {
        list->end->next = node;
        list->end = node;
    }
    list->list_size++;
    return;
}

void list_init(list_t *list) {
    list->list_size = 0;
    list->begin = NULL;
    list->end = NULL;
    return;
}

void list_clear(list_t *list) {
    int local_1c;
    node_t *next = list->begin;
    node_t *temp = NULL;
    for (int i = 0; i < list->list_size; i++) {
        if (next->next != NULL)
            temp = next->next;
        free(next->data);
        free(next);
        next = temp;
    }
    return;
}

int main() {
    char str[ARG_MAX + 1];
    list_t list;
    
    list_init(&list);
    printf("> ");
    fgets(str, ARG_MAX, stdin);
    while (str[0] != '.') {
        add_node(&list, str);
        printf("> ");
        fgets(str, ARG_MAX, stdin);
    }
    putchar('\n');

    node_t *node = list.begin;
    while (node != NULL) {
        printf("\"%s\"\n", node->data);
        node = node->next;
    }
    list_clear(&list);
    return 0;
}