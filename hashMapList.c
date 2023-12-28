#include <string.h>

#define INITIAL_LENGTH 100

typedef struct ListNode
{
    int value;
    struct ListNode *next;
} ListNode;

typedef struct
{
    char *key;
    ListNode *listNodes_head;
} List;

typedef struct
{
    List *list[INITIAL_LENGTH];
    int top;
} HashMapList;

void hashMapList_init(HashMapList *hashMapList)
{
    hashMapList->top = -1;

    for (int i = 0; i < INITIAL_LENGTH; i++)
    {
        hashMapList->list[i] = NULL;
    }
}

void list_free(List *list)
{
    ListNode *current = list->listNodes_head;
    ListNode *next = NULL;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    free(list->key);
    list = NULL;
}

void hashMapList_free(HashMapList *hashMapList)
{
    for (int i = 0; i < hashMapList->top; i++)
    {
        list_free(hashMapList->list[i]);
    }

    hashMapList = NULL;
}

List *get_list(HashMapList *hashMapList, const char *key)
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

void hashMapList_put(HashMapList *hashMapList, char *key, int value)
{
    if (hashMapList->top == -1 || get_list(hashMapList, key) == NULL)
    {
        List *new_list = hashMapList->list[hashMapList->top++];
        new_list = (List *)malloc(sizeof(List));

        new_list->key = (char *)malloc((strlen(key) + 1) * sizeof(char));
        strcpy(new_list->key, key);
        new_list->listNodes_head = NULL;

        ListNode *new_listNode = (ListNode *)malloc(sizeof(ListNode));
        new_listNode->value = value;
        new_listNode->next = new_list->listNodes_head;
        new_list->listNodes_head = new_listNode;

        hashMapList->list[hashMapList->top] = new_list;
    }
    else
    {
        List *list = get_list(hashMapList, key);

        if (list != NULL)
        {
            ListNode *new_listNode = (ListNode *)malloc(sizeof(ListNode));
            ;
            new_listNode->value = value;
            new_listNode->next = list->listNodes_head;
            list->listNodes_head = new_listNode;
        }
    }
}
