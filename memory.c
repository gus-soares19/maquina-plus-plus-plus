#include <string.h>

typedef struct TokenNode
{
    Token *token;
    int position;
    struct TokenNode *next;
} TokenNode;

typedef struct
{
    TokenNode *head;
    int position;

} Memory;

void memory_init(Memory *memory)
{
    memory->position = 0;
    memory->head = NULL;
}

void memory_free(Memory *memory)
{
    TokenNode *current = memory->head;
    TokenNode *next = NULL;

    while (current != NULL)
    {
        next = current->next;
        token_free(current->token);
        free(current);
        current = next;
    }

    memory = NULL;
}

TokenNode *get_token_at(int position, Memory *memory)
{
    if (memory->head == NULL)
    {
        return NULL;
    }

    TokenNode *tokenNode = memory->head;
    while (tokenNode->position < position && tokenNode != NULL)
    {
        tokenNode = tokenNode->next;
    }

    return tokenNode;
}

void add_token(Token *token, Memory *memory)
{
    TokenNode *new_tokenNode = (TokenNode *)malloc(sizeof(TokenNode));
    new_tokenNode->token = token;
    new_tokenNode->next = NULL;
    new_tokenNode->position = memory->position++;

    if (memory->head == NULL)
    {
        memory->head = new_tokenNode;
    }
    else
    {
        TokenNode *current = memory->head;
        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = new_tokenNode;
    }
}