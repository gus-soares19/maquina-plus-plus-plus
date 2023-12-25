#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct HashMapNode
{
    char *key;
    int value;
    struct HashMapNode *next;
} HashMapNode;

typedef struct
{
    HashMapNode *hashMapNodes_head;
} HashMap;

void hashMap_init(HashMap *hashMap)
{
    hashMap->hashMapNodes_head = NULL;
}

void hashMap_free(HashMap *hashMap)
{
    HashMapNode *current = hashMap->hashMapNodes_head;
    HashMapNode *next = NULL;

    while (current != NULL)
    {
        next = current->next;
        free(current->key);
        free(current);
        current = next;
    }

    hashMap = NULL;
}

void hashMap_put(HashMap *hashMap, const char *key, int value)
{
    HashMapNode *new_hashMapNode = (HashMapNode *)malloc(sizeof(HashMapNode));
    new_hashMapNode->key = (char *)malloc(16 * sizeof(char));

    strcpy(new_hashMapNode->key, key);
    new_hashMapNode->value = value;
    new_hashMapNode->next = NULL;

    if (hashMap->hashMapNodes_head == NULL)
    {
        hashMap->hashMapNodes_head = new_hashMapNode;
    }
    else
    {
        HashMapNode *current = hashMap->hashMapNodes_head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_hashMapNode;
    }
}

int hashMap_get(HashMap *hashMap, const char *key)
{
    HashMapNode *current = hashMap->hashMapNodes_head;
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            return current->value;
        }
        current = current->next;
    }
    return -1;
}
