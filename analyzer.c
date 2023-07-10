#include "lexical.c"
#include <string.h>

typedef struct {
    char *message;
    int code;
    char *type;
} HttpResponse;

HttpResponse* createHttpResponse(char* message, int code, char* type){
    HttpResponse* response = (HttpResponse*)malloc(sizeof(HttpResponse));

    response->code = code;
    response->type = type;
    response->message = message;

    return response;
}

HttpResponse* analyze(char* code) {
    str_replace(code, '+', ' ');
    str_replace(code, '_', '\n');
    str_replace(code, '-', ',');
    str_replace(code, '.', ';');
    str_replace(code, '*', ':');

    TokenList* tokenList = lex(code);
    char* message = (char*)malloc(1024 * sizeof(char));

    if (tokenList == NULL) {        
        return createHttpResponse("Nao foi possivel gerar a lista de tokens", 400, "Bad Request");
    }

    for (int i = 0; i < tokenList->count; i++) {
        Token* token = tokenList->tokens[i];
        if(token->type == 7){
            sprintf(message, "Token '%s' nao identificado encontrado na linha %d", token->value, token->line);
            return createHttpResponse(message, 400, "Bad Request");
        }
    }

    return createHttpResponse("OK", 200, "OK");
}

// Função auxiliar para liberar a memória dos tokens e da lista de tokens
void freeToken(Token* token) {
    free(token->value);
    free(token);
}

// Função para liberar a memória dos tokens e da lista de tokens
void freeTokens(TokenList* tokenList) {
    for (int i = 0; i < tokenList->count; i++) {
        freeToken(tokenList->tokens[i]);
    }
    free(tokenList->tokens);
    free(tokenList);
}