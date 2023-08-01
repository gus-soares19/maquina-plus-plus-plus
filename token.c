typedef struct
{
    int type;
    char *lexeme;
    int position;
} Token;

Token *createToken(int tokenType, char *lexeme, int startPosition)
{
    Token *token = (Token *)malloc(sizeof(Token));
    token->lexeme = (char *)malloc(128 * sizeof(char));

    token->type = tokenType;
    strcpy(token->lexeme, lexeme);
    token->position = startPosition;
    return token;
}

void freeToken(Token *token)
{
    if (token != NULL)
    {
        free(token->lexeme);
        free(token);
        token = NULL;
    }
}
