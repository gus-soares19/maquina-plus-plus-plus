#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "hashMap.c"
#include "hashMapList.c"

typedef struct
{
    Stack stack;
    HashMap labels;
    HashMapList labelsBuffer;
    int stackIndex;
    int generatedByte;
    int generatedByteAux;
    char *contextualizerError;
} Contextualizer;

void initializeContextualizer(Contextualizer *contextualizer)
{
    contextualizer->stackIndex = 0;
    contextualizer->generatedByte = 0;
    contextualizer->generatedByteAux = 0;
    contextualizer->contextualizerError = (char *)malloc(1024 * sizeof(char));

    strcpy(contextualizer->contextualizerError, "");

    initializeStack(&contextualizer->stack);
    initializeHashMap(&contextualizer->labels);
    initializeHashMapList(&contextualizer->labelsBuffer);
}

void freeContextualizer(Contextualizer *contextualizer)
{
    freeHashMap(&(contextualizer->labels));
    freeHashMapList(&(contextualizer->labelsBuffer));
    clear(&(contextualizer->stack));
    free(contextualizer->contextualizerError);

    contextualizer = NULL;
}

void toUpperCase(char *str)
{
    int i = 0;
    while (str[i] != '\0')
    {
        str[i] = toupper(str[i]);
        i++;
    }
}

void executeAction(Contextualizer *contextualizer, int action, Token *token)
{
    char *value = (char *)malloc(7 * sizeof(char));
    char *error = (char *)malloc(128 * sizeof(char));

    switch (action)
    {
    case PROGRAM_START:
        contextualizer->stackIndex = 1;
        pushAt(&contextualizer->stack, 0, FIRST_DUMMY_BYTE);
        pushAt(&contextualizer->stack, 1, SECOND_DUMMY_BYTE);
        break;

    case INSTRUCTION_START:
        contextualizer->generatedByte = 0;
        contextualizer->generatedByteAux = 0;
        break;

    case PROGRAM_END:
        set(&contextualizer->stack, 1, contextualizer->stackIndex - 1);

        int bufferSize = sizeof(contextualizer->labelsBuffer.list) / sizeof(contextualizer->labelsBuffer.list[0]);
        for (size_t i = 0; i < bufferSize; i++)
        {
            List *bufferList = contextualizer->labelsBuffer.list[i];

            if (bufferList != NULL)
            {
                if (get(&contextualizer->labels, bufferList->key) != -1)
                {
                    int command = get(&contextualizer->labels, bufferList->key) - 3;
                    List *list = getList(&contextualizer->labelsBuffer, bufferList->key);

                    if (list != NULL)
                    {
                        NodeList *current = list->values;
                        while (current != NULL)
                        {
                            pushAt(&contextualizer->stack, current->value, command >> _08);
                            pushAt(&contextualizer->stack, (current->value + 1), command & _FF);
                            current = current->next;
                        }
                    }
                }
                else
                {
                    strcpy(error, "");
                    sprintf(error, "%s%s", "Label does not exist: ", bufferList->key);
                    strcpy(contextualizer->contextualizerError, error);
                }
            }
        }

        break;

    case ADD:
        contextualizer->generatedByte &= _1F;
        break;

    case SUB:
        contextualizer->generatedByte &= _3F;
        contextualizer->generatedByte |= _20;
        break;

    case AND:
        contextualizer->generatedByte &= _5F;
        contextualizer->generatedByte |= _40;
        break;

    case OR:
        contextualizer->generatedByte &= _7F;
        contextualizer->generatedByte |= _60;
        break;

    case XOR:
        contextualizer->generatedByte &= _9F;
        contextualizer->generatedByte |= _80;
        break;

    case NOT:
        contextualizer->generatedByte &= _BF;
        contextualizer->generatedByte |= _A0;
        break;

    case MOV:
        contextualizer->generatedByte &= _DF;
        contextualizer->generatedByte |= _C0;
        break;

    case INC:
        contextualizer->generatedByte &= _FF;
        contextualizer->generatedByte |= _E0;
        break;

    case HD:
        contextualizer->stackIndex++;
        resize(&contextualizer->stack, contextualizer->stackIndex + 1);
        pushAt(&contextualizer->stack, contextualizer->stackIndex, _07);
        break;

    case AC_AC__ROM_AC__RET:
        contextualizer->generatedByte &= _F8;
        break;

    case AC_REG__ROM_REG__DRRAM_AC:
        contextualizer->generatedByte &= _F9;
        contextualizer->generatedByte |= _01;
        break;

    case AC_RAM__ROM_RAM__AC_DRAM:
        contextualizer->generatedByte &= _FA;
        contextualizer->generatedByte |= _02;
        break;

    case AC_OUT__JMP__PUSHREG:
        contextualizer->generatedByte &= _FB;
        contextualizer->generatedByte |= _03;
        break;

    case REG_AC__JMPC__POPREG:
        contextualizer->generatedByte &= _FC;
        contextualizer->generatedByte |= _04;
        break;

    case RAM_AC__JMPZ__PUSHA:
        contextualizer->generatedByte &= _FD;
        contextualizer->generatedByte |= _05;
        break;

    case IN_AC__CALL__POPA:
        contextualizer->generatedByte &= _FE;
        contextualizer->generatedByte |= _06;
        break;

    case DONT_USE:
        contextualizer->generatedByte &= _FF;
        contextualizer->generatedByte |= _07;
        break;

    case REG_B:
        contextualizer->generatedByte &= _E7;
        break;

    case REG_C:
        contextualizer->generatedByte &= _EF;
        contextualizer->generatedByte |= _08;
        break;

    case REG_D:
        contextualizer->generatedByte &= _F7;
        contextualizer->generatedByte |= _10;
        break;

    case REG_E:
        contextualizer->generatedByte &= _FF;
        contextualizer->generatedByte |= _18;
        break;

    case IN_OUT_FLAG:

        if (strcasecmp(IN0, token->lexeme) || strcasecmp(OUT0, token->lexeme))
        {
            contextualizer->generatedByte &= _E7;
        }
        else if (strcasecmp(IN1, token->lexeme) || strcasecmp(OUT1, token->lexeme))
        {
            contextualizer->generatedByte &= _EF;
            contextualizer->generatedByte |= _08;
        }
        else if (strcasecmp(IN2, token->lexeme) || strcasecmp(OUT2, token->lexeme))
        {
            contextualizer->generatedByte &= _F7;
            contextualizer->generatedByte |= _10;
        }
        else if (strcasecmp(IN3, token->lexeme) || strcasecmp(OUT3, token->lexeme))
        {
            contextualizer->generatedByte &= _FF;
            contextualizer->generatedByte |= _18;
        }

        break;

    case RAM_RECORD:
        strcpy(value, "");
        sprintf(value, "%s%s", HEXA_PREFIX, substring(token->lexeme, 1, 3));
        contextualizer->generatedByte = (int)strtol(value, NULL, 0);
        contextualizer->stackIndex++;
        resize(&contextualizer->stack, contextualizer->stackIndex + 1);
        pushAt(&contextualizer->stack, contextualizer->stackIndex, contextualizer->generatedByte);
        contextualizer->generatedByte = 0;
        break;

    case ROM_PREPARE:
        strcpy(value, "");
        sprintf(value, "%s%s", HEXA_PREFIX, token->lexeme);
        contextualizer->generatedByteAux = (int)strtol(value, NULL, 0);
        break;

    case AUX_RECORD:
        contextualizer->stackIndex++;
        resize(&contextualizer->stack, contextualizer->stackIndex + 1);
        pushAt(&contextualizer->stack, contextualizer->stackIndex, contextualizer->generatedByteAux);
        contextualizer->generatedByteAux = 0;
        break;

    case BYTE_RECORD:
        contextualizer->stackIndex++;
        resize(&contextualizer->stack, contextualizer->stackIndex + 1);
        pushAt(&contextualizer->stack, contextualizer->stackIndex, contextualizer->generatedByte);
        contextualizer->generatedByte = 0;
        break;

    case REND:
        strcpy(value, "");
        sprintf(value, "%s%s", HEXA_PREFIX, substring(token->lexeme, 1, 3));
        contextualizer->generatedByte = (int)strtol(value, NULL, 0);
        contextualizer->stackIndex++;
        resize(&contextualizer->stack, contextualizer->stackIndex + 1);
        pushAt(&contextualizer->stack, contextualizer->stackIndex, contextualizer->generatedByte);

        strcpy(value, "");
        sprintf(value, "%s%s", HEXA_PREFIX, substring(token->lexeme, 3, 5));
        contextualizer->generatedByte = (int)strtol(value, NULL, 0);
        contextualizer->stackIndex++;
        resize(&contextualizer->stack, contextualizer->stackIndex + 1);
        pushAt(&contextualizer->stack, contextualizer->stackIndex, contextualizer->generatedByte);
        break;

    case LABEL_TRANSLATE:
        toUpperCase(token->lexeme);
        insertValue(&contextualizer->labelsBuffer, token->lexeme, contextualizer->stackIndex + 1);
        contextualizer->stackIndex = contextualizer->stackIndex + 2;
        resize(&contextualizer->stack, contextualizer->stackIndex + 1);
        break;

    case LABEL_RECORD:
        toUpperCase(token->lexeme);
        char *label = substring(token->lexeme, 0, strlen(token->lexeme) - 1);

        if (get(&contextualizer->labels, label) == -1)
        {
            put(&contextualizer->labels, label, contextualizer->stackIndex + 2);
        }
        else
        {
            strcpy(error, "");
            sprintf(error, "%s%s", "Duplicated label: ", label);
            strcpy(contextualizer->contextualizerError, error);
        }
        free(label);
        break;

    default:
        break;
    }

    free(value);
    free(error);
}
