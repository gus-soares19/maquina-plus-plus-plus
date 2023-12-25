typedef struct
{
    int type;
    char *lexeme;
    int position;
} Token;

Token *token_create(int type, char *lexeme, int start)
{
    Token *token = (Token *)malloc(sizeof(Token));
    token->lexeme = (char *)malloc(16 * sizeof(char));

    strcpy(token->lexeme, lexeme);
    token->type = type;
    token->position = start;
    return token;
}

void token_free(Token *token)
{
    if (token != NULL)
    {
        free(token->lexeme);
        free(token);
        token = NULL;
    }
}
