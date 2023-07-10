#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>

typedef enum {
    TOKEN_LABEL, // 0
    TOKEN_INSTRUCTION, // 1
    TOKEN_REGISTER, // 2
    TOKEN_NUMBER, // 3
    TOKEN_SEMICOLON, // 4
    TOKEN_COMA, // 5
    TOKEN_NEWLINE, // 6
    TOKEN_INVALID // 7
} TokenType;

// Definição das estruturas de token
typedef struct {
    TokenType type;
    char* value;
    int line;
} Token;

typedef struct {
    Token** tokens;
    int count;
} TokenList;

// Função auxiliar para criar um novo token
Token* createToken(TokenType type, char* value, int line) {
    Token* token = (Token*)malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    token->line = line;
    return token;
}

// Função auxiliar para adicionar um token à lista
void addToken(TokenList* tokenList, Token* token) {
    tokenList->tokens = (Token**)realloc(tokenList->tokens, (tokenList->count + 1) * sizeof(Token*));
    tokenList->tokens[tokenList->count] = token;
    tokenList->count++;
}

void str_replace(char* str, char find, char replace) {
    if (str == NULL)
        return;

    int length = strlen(str);

    for (int i = 0; i < length; i++) {
        if (str[i] == find)
            str[i] = replace;
    }
}

// Função para realizar a análise léxica
TokenList* lex(const char* code) {
    TokenList* tokenList = (TokenList*)malloc(sizeof(TokenList));
    tokenList->tokens = NULL;
    tokenList->count = 0;

    int line = 1;
    int length = strlen(code);
    int i = 0;

    while (i < length) {
        if (isspace(code[i])) {
            if(code[i] == '\n'){
                char* value = (char*)malloc(2 * sizeof(char));
                value[0] = '\n';
                value[1] = '\0';
                addToken(tokenList, createToken(TOKEN_NEWLINE, value, line));
                line++;                
        }
        i++;
        continue;
        }

        if (isalpha(code[i])) {
            int start = i;
            while (isalnum(code[i])) {
                i++;
            }
            int len = i - start;
            char* value = (char*)malloc((len + 1) * sizeof(char));
            strncpy(value, &code[start], len);
            value[len] = '\0';

            if (code[i] == ':'){
                addToken(tokenList, createToken(TOKEN_LABEL, value, line));
                i++;
            }else if(tokenList->count > 0 && tokenList->tokens[tokenList->count - 1]->type == TOKEN_INSTRUCTION &&(
                    strcmp(tokenList->tokens[tokenList->count - 1]->value, "JMP") == 0) || 
                    strcmp(tokenList->tokens[tokenList->count - 1]->value, "CALL") == 0) {
                addToken(tokenList, createToken(TOKEN_LABEL, value, line));
            } else if (len == 1 && (code[start] == 'A' || code[start] == 'B' || code[start] == 'C' || 
                                    code[start] == 'D' || code[start] == 'E')) {
                addToken(tokenList, createToken(TOKEN_REGISTER, value, line));
            } else if (strncmp(value, "ADD", 3) == 0 || strncmp(value, "SUB", 3) == 0 || strncmp(value, "MOV", 3) == 0 ||
                       strncmp(value, "INC", 3) == 0 || strncmp(value, "JMP", 3) == 0 || strncmp(value, "CALL", 3) == 0 ||
                       strncmp(value, "RET", 3) == 0 || strncmp(value, "PUSH", 4) == 0 || strncmp(value, "POP", 3) == 0){
                addToken(tokenList, createToken(TOKEN_INSTRUCTION, value, line));
            }else{
                addToken(tokenList, createToken(TOKEN_INVALID, value, line));
            }
            continue;
        }

        // Verificar número
        if (isdigit(code[i])) {
            int start = i;
            while (isdigit(code[i])) {
                i++;
            }
            int len = i - start;
            char* value = (char*)malloc((len + 1) * sizeof(char));
            strncpy(value, &code[start], len);
            value[len] = '\0';
            addToken(tokenList, createToken(TOKEN_NUMBER, value, line));
            continue;
        }

        // Verificar ponto e virgula
        if (code[i] == ';') {
            char* value = (char*)malloc(2 * sizeof(char));
            value[0] = ';';
            value[1] = '\0';
            addToken(tokenList, createToken(TOKEN_SEMICOLON, value, line));
            i++;
            continue;
        }

        // Verificar virgula
        if (code[i] == ',') {
            char* value = (char*)malloc(2 * sizeof(char));
            value[0] = ',';
            value[1] = '\0';
            addToken(tokenList, createToken(TOKEN_COMA, value, line));
            i++;
            continue;
        }

        // Se chegou até aqui, trata-se de um caractere inválido
        char* value = (char*)malloc(2 * sizeof(char));
        value[0] = code[i];
        value[1] = '\0';
        addToken(tokenList, createToken(TOKEN_INVALID, value, line));
        i++;
    }

    return tokenList;
}
