#include <stdbool.h>
#include <ctype.h>

typedef struct
{
    bool passed;
    char* message;
} ParserResponse;

bool isValidLabel(Token* token, ParserResponse* response){
    if(!isalpha(token->value[0])){
        sprintf(response->message, "Label '%s' na linha %d deve iniciar com uma letra", token->value, token->line);
        response->passed = false;
        return false;
    }

    for (int i = 0; token->value[i] != '\0'; i++) {
        char c = token->value[i];
        if (!(isalnum(c) || c == '_')) {
            sprintf(response->message, "Label '%s' na linha %d deve conter apenas letras, numeros ou '_'", token->value, token->line);
            response->passed = false;
            return false;
        }
    }

    return true;
}

bool isValidIdentifier(Token* token, ParserResponse* response){
    if(!isalpha(token->value[0])){
        sprintf(response->message, "Label '%s' na linha %d deve iniciar com uma letra", token->value, token->line);
        response->passed = false;
        return false;
    }

    for (int i = 0; token->value[i] != '\0'; i++) {
        char c = token->value[i];
        if (!(isalnum(c) || c == '_')) {
            sprintf(response->message, "Label '%s' na linha %d deve conter apenas letras, numeros ou '_'", token->value, token->line);
            response->passed = false;
            return false;
        }
    }

    return true;
}

bool isValidInstruction(Token* token){
    return true;
}

bool isValidRegister(Token* token){
    return true;
}

bool isValidNumber(Token* token){
    return true;
}

ParserResponse* parse(TokenList* tokenList){
    ParserResponse* response = (ParserResponse*)malloc(sizeof(ParserResponse));

    for (int i = 0; i < tokenList->count; i++) {
        Token* token = tokenList->tokens[i];
        if(token->type == 0 || token->type == 1){
            isValidIdentifier(token, response);            
        }else if(token->type == 2 && !isValidInstruction(token)){

        }else if(token->type == 3 && !isValidRegister(token)){

        }else if (token->type == 4 && !isValidNumber(token)){

        } 
    }

    response->passed = true;
    response->message = "OK";
    return response;
}
