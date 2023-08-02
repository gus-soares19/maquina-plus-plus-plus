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

    strcpy(token->lexeme, lexeme);
    token->type = tokenType;
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
