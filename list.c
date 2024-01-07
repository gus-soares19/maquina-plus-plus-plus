#include <stdlib.h>

typedef struct
{
    int *data;
    size_t length;
} List;

void list_init(size_t length, List *list)
{
    list->data = (int *)malloc(length * sizeof(int));
    list->length = length;
}

void list_free(List *list)
{
    free(list->data);
    list->data = NULL;
    list->length = 0;
}

void add_number(int number, List *list)
{
    list->data = (int *)realloc(list->data, (list->length + 1) * sizeof(int));
    list->data[list->length++] = number;
}

int add_number_at(int number, int position, List *list)
{
    if (position >= list->length)
    {
        return -1;
    }

    list->data[position] = number;
    return number;
}

int get_number_at(int position, List *list)
{
    return position >= list->length ? -1 : list->data[position];
}
