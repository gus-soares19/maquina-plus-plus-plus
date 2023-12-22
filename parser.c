#include "token.c"
#include "stack.c"
#include "constants.c"
#include "tokenizer.c"
#include "contextualizer.c"
#include "machine.c"
#include <stdbool.h>

typedef struct
{
    Stack _stack;
    Token *_currentToken;
    Token *_previousToken;
    Tokenizer _tokenizer;
    Contextualizer _contextualizer;
    Machine _machine;
    char *parserError;
} Parser;

bool isTerminal(int x)
{
    return x < FIRST_NON_TERMINAL;
}

bool isNonTerminal(int x)
{
    return x >= FIRST_NON_TERMINAL && x < FIRST_SEMANTIC_ACTION;
}

bool pushProduction(Parser *parser, int topStack, int tokenInput)
{
    int p = PARSER_TABLE[topStack - FIRST_NON_TERMINAL][tokenInput - 1];

    if (p >= 0)
    {
        int size = sizeof(PRODUCTIONS) / sizeof(PRODUCTIONS[p]);
        int production[size];
        memcpy(production, PRODUCTIONS[p], size);

        for (int i = 7; i >= 0; i--)
        {
            if (production[i] != 0 || i == 0)
            {
                push(&(parser->_stack), production[i]);
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool isValid(Token *token)
{
    int size = sizeof(MEMORY_CASES_VALUES) / sizeof(MEMORY_CASES_VALUES[0]);
    for (size_t i = 0; i < size; i++)
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
    if (parser->_currentToken == NULL)
    {
        int pos = 0;
        if (parser->_previousToken != NULL)
        {
            pos = parser->_previousToken->position + strlen(parser->_previousToken->lexeme);
        }

        parser->_currentToken = createToken(DOLLAR, "$", pos);
    }

    int x = pop(&(parser->_stack));
    int a = parser->_currentToken->type;

    if (x == EPSILON)
    {
        return false;
    }
    else if (isTerminal(x))
    {
        if (x == a)
        {
            if (isEmpty(&(parser->_stack)))
            {
                return true;
            }
            else
            {
                parser->_previousToken = parser->_currentToken;
                parser->_currentToken = getNextToken(&(parser->_tokenizer));

                if (parser->_currentToken != NULL && isValid(parser->_currentToken))
                {
                    addToken(parser->_currentToken, &(parser->_machine.memory));

                    if (parser->_currentToken->type == 29)
                    {
                        addLabel(parser->_currentToken, &(parser->_machine));
                    }
                }

                return false;
            }
        }
        else
        {
            strcpy(parser->parserError, PARSER_ERROR[x]);
            return true;
        }
    }
    else if (isNonTerminal(x))
    {
        if (pushProduction(parser, x, a))
        {
            return false;
        }
        else
        {
            strcpy(parser->parserError, PARSER_ERROR[x]);
            return true;
        }
    }
    else
    {
        executeAction(&(parser->_contextualizer), x - FIRST_SEMANTIC_ACTION, parser->_previousToken);
        return false;
    }
}

void initializeParser(Parser *parser)
{
    initializeTokenizer(&(parser->_tokenizer));
    initializeContextualizer(&(parser->_contextualizer));
    initializeStack(&(parser->_stack));
    initializeMachine(&(parser->_machine));
    clear(&(parser->_stack));
    push(&(parser->_stack), DOLLAR);
    push(&(parser->_stack), START_SYMBOL);

    parser->parserError = (char *)malloc(192 * sizeof(char));

    strcpy(parser->parserError, "");

    parser->_currentToken = NULL;
    parser->_previousToken = NULL;
}

void freeParser(Parser *parser)
{
    freeMachine(&(parser->_machine));
    freeContextualizer(&(parser->_contextualizer));
    freeTokenizer(&(parser->_tokenizer));
    clear(&(parser->_stack));
    free(parser->parserError);
    free(parser);

    parser->parserError = NULL;
    parser = NULL;
}

HttpResponse *parse(Parser *parser, char *code, char *timer, char *delay)
{
    setInput(&(parser->_tokenizer), code);
    setTimer(&(parser->_machine), atoi(timer));
    setDelay(&(parser->_machine), strtod(delay, NULL));

    parser->_currentToken = getNextToken(&(parser->_tokenizer));

    if (parser->_currentToken != NULL && isValid(parser->_currentToken))
    {
        addToken(parser->_currentToken, &(parser->_machine.memory));

        if (parser->_currentToken->type == 29)
        {
            addLabel(parser->_currentToken, &(parser->_machine));
        }
    }

    while (!step(parser))
    {
        if (strlen(parser->_tokenizer.tokenizerError) != 0 ||
            strlen(parser->_contextualizer.contextualizerError) != 0 ||
            strlen(parser->parserError) != 0)
        {
            break;
        };
    }

    if (strlen(parser->_tokenizer.tokenizerError) != 0)
    {
        return createHttpResponse(parser->_tokenizer.tokenizerError, 400, "Bad Request");
    }
    if (strlen(parser->parserError) != 0)
    {
        return createHttpResponse(parser->parserError, 400, "Bad Request");
    }
    if (strlen(parser->_contextualizer.contextualizerError) != 0)
    {
        return createHttpResponse(parser->_contextualizer.contextualizerError, 400, "Bad Request");
    }

    return execute(&(parser->_machine));
}
