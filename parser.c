#include "token.c"
#include "stack.c"
#include "constants.c"
#include "tokenizer.c"
#include "contextualizer.c"
#include <stdbool.h>

typedef struct
{
    Stack _stack;
    Token *_currentToken;
    Token *_previousToken;
    Tokenizer _tokenizer;
    Contextualizer _contextualizer;
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

void replace(char *str)
{
    if (str == NULL)
    {
        return;
    }

    char find[5] = {'*', '+', '_', '-', '.'};
    char replace[5] = {':', ' ', '\n', ',', ';'};

    int length = strlen(str);
    int findSize = sizeof(find) / sizeof(find[0]);

    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < findSize; j++)
        {
            if (str[i] == find[j])
            {
                str[i] = replace[j];
                break;
            }
        }
    }
}

void initializeParser(Parser *parser)
{
    initializeTokenizer(&(parser->_tokenizer));
    initializeContextualizer(&(parser->_contextualizer));
    initializeStack(&(parser->_stack));
    clear(&(parser->_stack));
    push(&(parser->_stack), DOLLAR);
    push(&(parser->_stack), START_SYMBOL);

    parser->parserError = (char *)malloc(1024 * sizeof(char));

    strcpy(parser->parserError, "");

    parser->_currentToken = NULL;
    parser->_previousToken = NULL;
}

void freeParser(Parser *parser)
{
    freeContextualizer(&(parser->_contextualizer));
    freeTokenizer(&(parser->_tokenizer));
    clear(&(parser->_stack));
    freeToken(parser->_previousToken);
    freeToken(parser->_currentToken);
    free(parser->parserError);
    free(parser);
    parser = NULL;
}

HttpResponse *parse(Parser *parser, char *code)
{
    replace(code);
    setInput(&(parser->_tokenizer), code);

    parser->_currentToken = getNextToken(&(parser->_tokenizer));

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

    return createHttpResponse("Compilado com suscesso", 200, "OK");
}