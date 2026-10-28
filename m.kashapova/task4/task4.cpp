#include <iostream>
#include <cstdlib>
#include <cstring>
#define BUFFER_SIZE 1024

struct Node {
    char* text;
    struct Node* next;
};

int main() {
    struct Node* head = nullptr;
    struct Node* tail = nullptr;
    char buffer[BUFFER_SIZE];

    std::cout << "Вводите строки (для завершения введите точку '.' в начале строки):\n";

    while (fgets(buffer, sizeof(buffer), stdin) != nullptr) {
        if (buffer[0] == '.') {
            break;
        }

        
        if (strchr(buffer, '\x1b') != nullptr) {
            continue;
        }
       

        size_t len = strlen(buffer);

        struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
        if (newNode == nullptr) {
            std::cerr << "Ошибка: не удалось выделить память под узел списка!\n";
            return 1;
        }

        newNode->text = (char*)malloc((len + 1) * sizeof(char));
        if (newNode->text == nullptr) {
            std::cerr << "Ошибка: не удалось выделить память под строку!\n";
            free(newNode);
            return 1;
        }

        strcpy(newNode->text, buffer);
        newNode->next = nullptr;

        if (head == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    std::cout << "\nСписок\n";
    struct Node* current = head;
    while (current != nullptr) {
        std::cout << current->text;
        current = current->next;
    }

    current = head;
    while (current != nullptr) {
        struct Node* nextNode = current->next;
        free(current->text);
        free(current);
        current = nextNode;
    }

    return 0;
}
