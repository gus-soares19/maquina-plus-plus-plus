#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "tokenizerConstants.h"
#include "list.c"

typedef struct
{
    int position;
    char *input;
    char *error;
    List lines_length;

} Tokenizer;

void tokenizer_init(Tokenizer *tokenizer)
{
    tokenizer->position = 0;
    tokenizer->input = NULL;
    tokenizer->error = NULL;

    list_init(1, &(tokenizer->lines_length));
}

void tokenizer_free(Tokenizer *tokenizer)
{
    if (tokenizer->input != NULL)
    {
        free(tokenizer->input);
    }

    if (tokenizer->error != NULL)
    {
        free(tokenizer->error);
    }

    list_free(&(tokenizer->lines_length));
    tokenizer = NULL;
}

void set_input(Tokenizer *tokenizer, char *input)
{
    tokenizer->input = (char *)malloc((strlen(input) + 1) * sizeof(char));
    strcpy(tokenizer->input, input);
}

int get_next_state(unsigned char c, int state)
{
    int next = SCANNER_TABLE[state][c];
    return next;
}

void set_lines_length(Tokenizer *tokenizer)
{
    char input_cpy[strlen(tokenizer->input) + 1];
    strcpy(input_cpy, tokenizer->input);

    char *line = strtok(input_cpy, "\n");
    int count = 0;

    while (line != NULL)
    {
        // adiciona no comprimento da linha atual o tamanho da linha anterior
        size_t length = strlen(line) + (count == 0 ? 0 : get_number_at((count - 1), &(tokenizer->lines_length)));
        add_number((int)length, &(tokenizer->lines_length));

        line = strtok(NULL, "\n");
        count++;
    }
}

int get_line_by_position(int position, Tokenizer *tokenizer)
{
    int closest_higher = INT_MAX, line = 0;

    for (size_t i = 0; i < tokenizer->lines_length.length; ++i)
    {
        if (tokenizer->lines_length.data[i] >= position && tokenizer->lines_length.data[i] < closest_higher)
        {
            closest_higher = tokenizer->lines_length.data[i];
            line = i;
        }
    }
    return closest_higher == INT_MAX? tokenizer->lines_length.length : (line + 1);
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
        int line = get_line_by_position(start, tokenizer); // obtem a linha do token

        // monta a mensagem de erro
        int length = snprintf(NULL, 0, "Erro na linha %d: %s.", line, SCANNER_ERROR[last_state]) + 1;
        tokenizer->error = (char *)malloc(length * sizeof(char));

        sprintf(tokenizer->error, "Erro na linha %d: %s.", line, SCANNER_ERROR[last_state]);
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
