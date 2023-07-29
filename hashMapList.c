#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct NodeList {
    int value;
    struct NodeList* next;
} NodeList;

typedef struct {
    char *key;
    NodeList* values;
} List;

typedef struct {
    List list[100];
    int top;
} HashMapList;

// int hashFunction(const char* key) {
//     int sum = 0;
//     for (int i = 0; key[i] != '\0'; i++) {
//         sum += key[i];
//     }
//     return sum % 100;
// }

void initializeHashMapList(HashMapList* hashMapList) {
    hashMapList = (HashMapList*)malloc(sizeof(HashMapList));
    hashMapList->top = -1;
}

void insertValue(HashMapList* hashMapList, char* key, int value) { 
    List* list = &(hashMapList->list[hashMapList->top++]);

    // Verifica se a chave já existe na tabela
    if (list->key != NULL && strcmp(list->key, key) != 0) {
        strcpy(list->key, key);
        list->values = NULL;
    }
    else
    {
        list->key = key;
    }

    // Adiciona o valor à lista
    NodeList* newNodeList = (NodeList*)malloc(sizeof(NodeList));
    newNodeList->value = value;
    newNodeList->next = list->values;
    list->values = newNodeList;
}

List* getList(HashMapList* hashMapList, const char* key)
{
for (int i = 0; i < 100; i++) {
        List* list = &(hashMapList->list[i]);
        if (list->key != NULL && strcmp(list->key, key) == 0) {
            return list;
        }
    }

    return NULL;
}

int getListSize(List* list) {
    if(list == NULL){
        return 0;
    }

    int size = 0;
    NodeList* current = list->values;

    while (current != NULL) {
        size++;
        current = current->next;
    }

    return size;
}
