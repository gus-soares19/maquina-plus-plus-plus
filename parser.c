#include "token.c"
#include "stack.c"
#include "constants.c"
#include "tokenizer.c"
#include "contextualizer.c"
#include "machine.c"
#include <stdbool.h>

typedef struct
{
    Stack stack;
    Token *current_token;
    Token *previous_token;
    Tokenizer tokenizer;
    Contextualizer contextualizer;
    Machine machine;
    char *error;
} Parser;

bool is_terminal(int x)
{
    return x < FIRST_NON_TERMINAL;
}

bool is_non_terminal(int x)
{
    return x >= FIRST_NON_TERMINAL && x < FIRST_SEMANTIC_ACTION;
}

bool push_production(Parser *parser, int topStack, int tokenInput)
{
    int position = PARSER_TABLE[topStack - FIRST_NON_TERMINAL][tokenInput - 1];

    if (position >= 0)
    {
        int length = sizeof(PRODUCTIONS) / sizeof(PRODUCTIONS[position]);
        int production[length];
        memcpy(production, PRODUCTIONS[position], length);

        for (int i = 7; i >= 0; i--)
        {
            if (production[i] != 0 || i == 0)
            {
                push(&(parser->stack), production[i]);
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool is_valid(Token *token)
{
    int length = sizeof(MEMORY_CASES_VALUES) / sizeof(MEMORY_CASES_VALUES[0]);
    for (size_t i = 0; i < length; i++)
    {
        if (token->type == MEMORY_CASES_VALUES[i])
        {
            return true;
        }
    }

    return false;
}

bool step(Parser *parser)
{
    if (parser->current_token == NULL)
    {
        int position = 0;
        if (parser->previous_token != NULL)
        {
            position = parser->previous_token->position + strlen(parser->previous_token->lexeme);
        }

        parser->current_token = token_create(DOLLAR, "$", position);
    }

    int x = pop(&(parser->stack));
    int a = parser->current_token->type;

    if (x == EPSILON)
    {
        return false;
    }
    else if (is_terminal(x))
    {
        if (x == a)
        {
            if (is_empty(&(parser->stack)))
            {
                return true;
            }
            else
            {
                parser->previous_token = parser->current_token;
                parser->current_token = get_next_token(&(parser->tokenizer));

                if (parser->current_token != NULL && is_valid(parser->current_token))
                {
                    add_token(parser->current_token, &(parser->machine.memory));

                    if (parser->current_token->type == 29)
                    {
                        add_label(parser->current_token, &(parser->machine));
                    }
                }

                return false;
            }
        }
        else
        {
            sprintf(parser->error, "Erro na posição %d: %s.", parser->current_token->position, PARSER_ERROR[x]);
            return true;
        }
    }
    else if (is_non_terminal(x))
    {
        if (push_production(parser, x, a))
        {
            return false;
        }
        else
        {
            strcpy(parser->error, PARSER_ERROR[x]);
            return true;
        }
    }
    else
    {
        execute_action(&(parser->contextualizer), x - FIRST_SEMANTIC_ACTION, parser->previous_token);
        return false;
    }
}

void parser_init(Parser *parser)
{
    tokenizer_init(&(parser->tokenizer));
    contextualizer_init(&(parser->contextualizer));
    stack_init(&(parser->stack));
    machine_init(&(parser->machine));

    parser->error = (char *)malloc(192 * sizeof(char));

    strcpy(parser->error, "");

    parser->current_token = NULL;
    parser->previous_token = NULL;
}

void parser_free(Parser *parser)
{
    machine_free(&(parser->machine));
    contextualizer_free(&(parser->contextualizer));
    tokenizer_free(&(parser->tokenizer));
    stack_free(&(parser->stack));
    free(parser->error);
    free(parser);

    parser->error = NULL;
    parser = NULL;
}

HttpResponse *parse(Parser *parser, char *code, int timer, double delay, int mode)
{
    set_input(&(parser->tokenizer), code);
    set_timer(&(parser->machine), timer);
    set_delay(&(parser->machine), delay);

    push(&(parser->stack), DOLLAR);
    push(&(parser->stack), START_SYMBOL);

    parser->current_token = get_next_token(&(parser->tokenizer));

    if (parser->current_token != NULL && is_valid(parser->current_token))
    {
        add_token(parser->current_token, &(parser->machine.memory));

        if (parser->current_token->type == 29)
        {
            add_label(parser->current_token, &(parser->machine));
        }
    }

    while (!step(parser))
    {
        if (strlen(parser->tokenizer.error) != 0 ||
            strlen(parser->contextualizer.error) != 0 ||
            strlen(parser->error) != 0)
        {
            break;
        };
    }

    if (strlen(parser->tokenizer.error) != 0)
    {
        return httpResponse_create(parser->tokenizer.error, 400, "Bad Request");
    }
    if (strlen(parser->error) != 0)
    {
        return httpResponse_create(parser->error, 400, "Bad Request");
    }
    if (strlen(parser->contextualizer.error) != 0)
    {
        return httpResponse_create(parser->contextualizer.error, 400, "Bad Request");
    }

    return mode == 0 ? httpResponse_create("Compilado com sucesso.", 200, "OK") : execute(&(parser->machine));
}
