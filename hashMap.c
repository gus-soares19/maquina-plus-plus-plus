#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Node {
    char* key;
    int value;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
} HashMap;

void initializeHashMap(HashMap* hashMap) {
    hashMap = (HashMap*)malloc(sizeof(HashMap));
    hashMap->head = NULL;
}

void put(HashMap* hashMap, const char* key, int value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->key = malloc(1024 * sizeof(char));
    strcpy(newNode->key, key);
    newNode->value = value;
    newNode->next = NULL;

    if (hashMap->head == NULL) {
        hashMap->head = newNode;
    } else {
        Node* current = hashMap->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

int get(HashMap* hashMap, const char* key) {
    Node* current = hashMap->head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return -1;  // Valor padrão para indicar chave não encontrada
}
