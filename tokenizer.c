#include <stdbool.h>
#include <string.h>

typedef struct
{
    int position;
    char *input;
    char *tokenizerError;
} Tokenizer;

void initializeTokenizer(Tokenizer *tokenizer)
{
    tokenizer->position = 0;
    tokenizer->input = (char *)malloc(1024 * sizeof(char));
    tokenizer->tokenizerError = (char *)malloc(32 * sizeof(char));

    strcpy(tokenizer->input, "");
    strcpy(tokenizer->tokenizerError, "");
}

void freeTokenizer(Tokenizer *tokenizer)
{
    free(tokenizer->input);
    free(tokenizer->tokenizerError);
    tokenizer = NULL;
}

void setPosition(Tokenizer *tokenizer, int pos)
{
    tokenizer->position = pos;
}

void setInput(Tokenizer *tokenizer, char *input)
{
    strcpy(tokenizer->input, input);
    setPosition(tokenizer, 0);
}

int getNextState(unsigned char c, int state)
{
    int next = SCANNER_TABLE[state][c];
    return next;
}

int getTokenForState(int state)
{
    if (state < 0 || state >= (sizeof(TOKEN_STATE) / sizeof(TOKEN_STATE[0])))
    {
        return -1;
    }

    return TOKEN_STATE[state];
}

int lookupToken(int base, const char *key)
{
    int start = SPECIAL_CASES_INDEXES[base];
    int end = SPECIAL_CASES_INDEXES[base + 1] - 1;

    while (start <= end)
    {
        int half = (start + end) / 2;
        int comp = strcmp(SPECIAL_CASES_KEYS[half], key);

        if (comp == 0)
        {
            return SPECIAL_CASES_VALUES[half];
        }
        else if (comp < 0)
        {
            start = half + 1;
        }
        else
        {
            end = half - 1;
        }
    }

    return base;
}

bool hasInput(Tokenizer *tokenizer)
{
    return tokenizer->position < strlen(tokenizer->input);
}

char getNextChar(Tokenizer *tokenizer)
{
    if (hasInput(tokenizer))
    {
        return tokenizer->input[tokenizer->position++];
    }
    else
    {
        return (char)-1;
    }
}

char *substring(const char *str, int start, int end)
{
    int substringLength = end - start;
    char *result = (char *)malloc((substringLength + 1) * sizeof(char));
    if (result == NULL)
    {
        return NULL;
    }

    strncpy(result, str + start, substringLength);
    result[substringLength] = '\0';

    return result;
}

Token *getNextToken(Tokenizer *tokenizer)
{
    if (!hasInput(tokenizer))
    {
        return NULL;
    }

    int start = tokenizer->position;

    int state = 0;
    int lastState = 0;
    int endState = -1;
    int end = -1;

    while (hasInput(tokenizer))
    {
        lastState = state;
        state = getNextState(getNextChar(tokenizer), state);

        if (state < 0)
        {
            break;
        }
        else
        {
            if (getTokenForState(state) >= 0)
            {
                endState = state;
                end = tokenizer->position;
            }
        }
    }

    if (endState < 0 || (endState != state && getTokenForState(lastState) == -2))
    {
        strcpy(tokenizer->tokenizerError, SCANNER_ERROR[lastState]);
        return NULL;
    }

    tokenizer->position = end;
    int token = getTokenForState(endState);

    if (token == 0)
    {
        return getNextToken(tokenizer);
    }
    else
    {
        char *lexeme = substring(tokenizer->input, start, end);
        token = lookupToken(token, lexeme);
        return createToken(token, lexeme, start);
    }
}
