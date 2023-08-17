#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INITIAL_SIZE 100

typedef struct NodeList
{
    int value;
    struct NodeList *next;
} NodeList;

typedef struct
{
    char *key;
    NodeList *values;
} List;

typedef struct
{
    List *list[INITIAL_SIZE];
    int top;
} HashMapList;

void initializeHashMapList(HashMapList *hashMapList)
{
    hashMapList->top = -1;

    for (int i = 0; i < INITIAL_SIZE; i++)
    {
        hashMapList->list[i] = NULL;
    }
}

void freeListNode(NodeList *node)
{
    if (node == NULL)
    {
        return;
    }
    free(node);
    node = NULL;
}

void freeList(List *list)
{
    if (list == NULL)
    {
        return;
    }

    freeListNode(list->values);
    list = NULL;
}

void freeHashMapList(HashMapList *hashMapList)
{
    for (int i = 0; i < hashMapList->top; i++)
    {
        freeList(hashMapList->list[i]);
    }

    hashMapList = NULL;
}

List *getList(HashMapList *hashMapList, const char *key)
{
    for (int i = 0; i < 100; i++)
    {
        List *list = hashMapList->list[i];
        if (list != NULL)
        {
            if (list->key != NULL && strcmp(list->key, key) == 0)
            {
                return list;
            }
        }
    }
    return NULL;
}

void insertValue(HashMapList *hashMapList, char *key, int value)
{
    if (hashMapList->top == -1 || getList(hashMapList, key) == NULL)
    {
        List *newList = hashMapList->list[hashMapList->top++];
        newList = (List *)malloc(sizeof(List));

        newList->key = (char *)malloc(16 * sizeof(char));
        strcpy(newList->key, key);
        newList->values = NULL;

        NodeList *newNodeList = (NodeList *)malloc(sizeof(NodeList));
        newNodeList->value = value;
        newNodeList->next = newList->values;
        newList->values = newNodeList;

        hashMapList->list[hashMapList->top] = newList;
    }
    else
    {
        List *list = getList(hashMapList, key);

        if (list != NULL)
        {
            NodeList *newNodeList = (NodeList *)malloc(sizeof(NodeList));
            ;
            newNodeList->value = value;
            newNodeList->next = list->values;
            list->values = newNodeList;
        }
    }
}

int getListSize(List *list)
{
    if (list == NULL)
    {
        return 0;
    }

    int size = 0;
    NodeList *current = list->values;

    while (current != NULL)
    {
        size++;
        current = current->next;
    }

    return size;
}
