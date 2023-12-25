#include <stdbool.h>
#include <string.h>

typedef struct
{
    int position;
    char *input;
    char *error;
} Tokenizer;

void tokenizer_init(Tokenizer *tokenizer)
{
    tokenizer->position = 0;
    tokenizer->input = (char *)malloc(1024 * sizeof(char));
    tokenizer->error = (char *)malloc(192 * sizeof(char));

    strcpy(tokenizer->input, "");
    strcpy(tokenizer->error, "");
}

void tokenizer_free(Tokenizer *tokenizer)
{
    free(tokenizer->input);
    free(tokenizer->error);

    tokenizer = NULL;
}

void set_position(Tokenizer *tokenizer, int pos)
{
    tokenizer->position = pos;
}

void set_input(Tokenizer *tokenizer, char *input)
{
    strcpy(tokenizer->input, input);
    set_position(tokenizer, 0);
}

int get_next_state(unsigned char c, int state)
{
    int next = SCANNER_TABLE[state][c];
    return next;
}

int get_token_by_state(int state)
{
    if (state < 0 || state >= (sizeof(TOKEN_STATE) / sizeof(TOKEN_STATE[0])))
    {
        return -1;
    }

    return TOKEN_STATE[state];
}

int lookup_token(int base, const char *key)
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

bool has_input(Tokenizer *tokenizer)
{
    return tokenizer->position < strlen(tokenizer->input);
}

char get_next_char(Tokenizer *tokenizer)
{
    if (has_input(tokenizer))
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
    int substring_length = end - start;
    char *result = (char *)malloc((substring_length + 1) * sizeof(char));
    if (result == NULL)
    {
        return NULL;
    }

    strncpy(result, str + start, substring_length);
    result[substring_length] = '\0';

    return result;
}

Token *get_next_token(Tokenizer *tokenizer)
{
    if (!has_input(tokenizer))
    {
        return NULL;
    }

    int start = tokenizer->position;

    int state = 0;
    int last_state = 0;
    int end_state = -1;
    int end = -1;

    while (has_input(tokenizer))
    {
        last_state = state;
        state = get_next_state(get_next_char(tokenizer), state);

        if (state < 0)
        {
            break;
        }
        else
        {
            if (get_token_by_state(state) >= 0)
            {
                end_state = state;
                end = tokenizer->position;
            }
        }
    }

    if (end_state < 0 || (end_state != state && get_token_by_state(last_state) == -2))
    {
        sprintf(tokenizer->error, "Erro na posição %d: %s.", start, SCANNER_ERROR[last_state]);
        return NULL;
    }

    tokenizer->position = end;
    int token = get_token_by_state(end_state);

    if (token == 0)
    {
        return get_next_token(tokenizer);
    }
    else
    {
        char *lexeme = substring(tokenizer->input, start, end);
        token = lookup_token(token, lexeme);
        return token_create(token, lexeme, start);
    }
}
