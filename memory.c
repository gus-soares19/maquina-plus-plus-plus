#include <string.h>

typedef struct TokenNode
{
    Token *token;
    int pos;
    struct TokenNode *next;
} TokenNode;

typedef struct
{
    TokenNode *head;
    int pos;

} Memory;

void initializeMemory(Memory *memory)
{
    memory->pos = 0;
    memory->head = NULL;
}

TokenNode *getTokenAt(int pos, Memory *memory)
{
    if (memory->head == NULL)
    {
        return NULL;
    }

    TokenNode *tokenNode = memory->head;
    while (tokenNode->pos < pos && tokenNode != NULL)
    {
        tokenNode = tokenNode->next;
    }

    return tokenNode;
}

void addToken(Token *token, Memory *memory)
{
    TokenNode *newNode = (TokenNode *)malloc(sizeof(TokenNode));
    newNode->token = token;
    newNode->next = NULL;
    newNode->pos = memory->pos++;

    if (memory->head == NULL)
    {
        memory->head = newNode;
    }
    else
    {
        TokenNode *current = memory->head;
        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = newNode;
    }
}