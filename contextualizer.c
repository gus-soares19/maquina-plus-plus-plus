#include <string.h>
#include "contextualizerConstants.h"
#include "hashMap.c"
#include "hashMapList.c"

typedef struct
{
    Stack stack;
    HashMap labels;
    HashMapList labels_buffer;
    int stack_index;
    int generated_byte;
    int generated_byte_aux;
    char *error;
} Contextualizer;

void contextualizer_init(Contextualizer *contextualizer)
{
    contextualizer->stack_index = 0;
    contextualizer->generated_byte = 0;
    contextualizer->generated_byte_aux = 0;
    contextualizer->error = NULL;

    stack_init(&contextualizer->stack);
    hashMap_init(&contextualizer->labels);
    hashMapList_init(&contextualizer->labels_buffer);
}

void contextualizer_free(Contextualizer *contextualizer)
{
    hashMap_free(&(contextualizer->labels));
    hashMapList_free(&(contextualizer->labels_buffer));
    stack_free(&(contextualizer->stack));

    if (contextualizer->error != NULL)
    {
        free(contextualizer->error);
    }

    contextualizer = NULL;
}

void to_upper_case(char *str)
{
    int i = 0;
    while (str[i] != '\0')
    {
        str[i] = toupper(str[i]);
        i++;
    }
}

void execute_action(Contextualizer *contextualizer, int action, Token *token)
{
    char *value = (char *)malloc(7 * sizeof(char));

    switch (action)
    {
    case PROGRAM_START:
        contextualizer->stack_index = 1;
        push_at(&contextualizer->stack, 0, FIRST_DUMMY_BYTE);
        push_at(&contextualizer->stack, 1, SECOND_DUMMY_BYTE);
        break;

    case INSTRUCTION_START:
        contextualizer->generated_byte = 0;
        contextualizer->generated_byte_aux = 0;
        break;

    case PROGRAM_END:
        push_at(&contextualizer->stack, 1, contextualizer->stack_index - 1);

        int bufferSize = sizeof(contextualizer->labels_buffer.list) / sizeof(contextualizer->labels_buffer.list[0]);
        for (size_t i = 0; i < bufferSize; i++)
        {
            List *bufferList = contextualizer->labels_buffer.list[i];

            if (bufferList != NULL)
            {
                if (hashMap_get(&(contextualizer->labels), bufferList->key) != -1)
                {
                    int command = hashMap_get(&(contextualizer->labels), bufferList->key) - 3;
                    List *list = get_list(&(contextualizer->labels_buffer), bufferList->key);

                    if (list != NULL)
                    {
                        ListNode *current = list->listNodes_head;
                        while (current != NULL)
                        {
                            push_at(&contextualizer->stack, current->value, command >> _08);
                            push_at(&contextualizer->stack, (current->value + 1), command & _FF);
                            current = current->next;
                        }
                    }
                }
                else
                {
                    int length = snprintf(NULL, 0, "Erro na posição %d: label '%s' não existe.", token->position, bufferList->key) + 1;
                    contextualizer->error = (char *)malloc(length * sizeof(char));

                    sprintf(contextualizer->error, "Erro na posição %d: label '%s' não existe.", token->position, bufferList->key);
                }
            }
        }

        break;

    case ADD:
        contextualizer->generated_byte &= _1F;
        break;

    case SUB:
        contextualizer->generated_byte &= _3F;
        contextualizer->generated_byte |= _20;
        break;

    case AND:
        contextualizer->generated_byte &= _5F;
        contextualizer->generated_byte |= _40;
        break;

    case OR:
        contextualizer->generated_byte &= _7F;
        contextualizer->generated_byte |= _60;
        break;

    case XOR:
        contextualizer->generated_byte &= _9F;
        contextualizer->generated_byte |= _80;
        break;

    case NOT:
        contextualizer->generated_byte &= _BF;
        contextualizer->generated_byte |= _A0;
        break;

    case MOV:
        contextualizer->generated_byte &= _DF;
        contextualizer->generated_byte |= _C0;
        break;

    case INC:
        contextualizer->generated_byte &= _FF;
        contextualizer->generated_byte |= _E0;
        break;

    case HD:
        contextualizer->stack_index++;
        resize(&contextualizer->stack, contextualizer->stack_index + 1);
        push_at(&contextualizer->stack, contextualizer->stack_index, _07);
        break;

    case AC_AC__ROM_AC__RET:
        contextualizer->generated_byte &= _F8;
        break;

    case AC_REG__ROM_REG__DRRAM_AC:
        contextualizer->generated_byte &= _F9;
        contextualizer->generated_byte |= _01;
        break;

    case AC_RAM__ROM_RAM__AC_DRAM:
        contextualizer->generated_byte &= _FA;
        contextualizer->generated_byte |= _02;
        break;

    case AC_OUT__JMP__PUSHREG:
        contextualizer->generated_byte &= _FB;
        contextualizer->generated_byte |= _03;
        break;

    case REG_AC__JMPC__POPREG:
        contextualizer->generated_byte &= _FC;
        contextualizer->generated_byte |= _04;
        break;

    case RAM_AC__JMPZ__PUSHA:
        contextualizer->generated_byte &= _FD;
        contextualizer->generated_byte |= _05;
        break;

    case IN_AC__CALL__POPA:
        contextualizer->generated_byte &= _FE;
        contextualizer->generated_byte |= _06;
        break;

    case DONT_USE:
        contextualizer->generated_byte &= _FF;
        contextualizer->generated_byte |= _07;
        break;

    case REG_B:
        contextualizer->generated_byte &= _E7;
        break;

    case REG_C:
        contextualizer->generated_byte &= _EF;
        contextualizer->generated_byte |= _08;
        break;

    case REG_D:
        contextualizer->generated_byte &= _F7;
        contextualizer->generated_byte |= _10;
        break;

    case REG_E:
        contextualizer->generated_byte &= _FF;
        contextualizer->generated_byte |= _18;
        break;

    case IN_OUT_FLAG:

        if (strcasecmp(IN0, token->lexeme) || strcasecmp(OUT0, token->lexeme))
        {
            contextualizer->generated_byte &= _E7;
        }
        else if (strcasecmp(IN1, token->lexeme) || strcasecmp(OUT1, token->lexeme))
        {
            contextualizer->generated_byte &= _EF;
            contextualizer->generated_byte |= _08;
        }
        else if (strcasecmp(IN2, token->lexeme) || strcasecmp(OUT2, token->lexeme))
        {
            contextualizer->generated_byte &= _F7;
            contextualizer->generated_byte |= _10;
        }
        else if (strcasecmp(IN3, token->lexeme) || strcasecmp(OUT3, token->lexeme))
        {
            contextualizer->generated_byte &= _FF;
            contextualizer->generated_byte |= _18;
        }

        break;

    case RAM_RECORD:
        strcpy(value, "");
        snprintf(value, strlen(value), "%s%s", HEXA_PREFIX, substring(token->lexeme, 1, 3));
        contextualizer->generated_byte = (int)strtol(value, NULL, 0);
        contextualizer->stack_index++;
        resize(&contextualizer->stack, contextualizer->stack_index + 1);
        push_at(&contextualizer->stack, contextualizer->stack_index, contextualizer->generated_byte);
        contextualizer->generated_byte = 0;
        break;

    case ROM_PREPARE:
        strcpy(value, "");
        snprintf(value, strlen(value), "%s%s", HEXA_PREFIX, token->lexeme);
        contextualizer->generated_byte_aux = (int)strtol(value, NULL, 0);
        break;

    case AUX_RECORD:
        contextualizer->stack_index++;
        resize(&contextualizer->stack, contextualizer->stack_index + 1);
        push_at(&contextualizer->stack, contextualizer->stack_index, contextualizer->generated_byte_aux);
        contextualizer->generated_byte_aux = 0;
        break;

    case BYTE_RECORD:
        contextualizer->stack_index++;
        resize(&contextualizer->stack, contextualizer->stack_index + 1);
        push_at(&contextualizer->stack, contextualizer->stack_index, contextualizer->generated_byte);
        contextualizer->generated_byte = 0;
        break;

    case REND:
        strcpy(value, "");
        snprintf(value, strlen(value), "%s%s", HEXA_PREFIX, substring(token->lexeme, 1, 3));
        contextualizer->generated_byte = (int)strtol(value, NULL, 0);
        contextualizer->stack_index++;
        resize(&contextualizer->stack, contextualizer->stack_index + 1);
        push_at(&contextualizer->stack, contextualizer->stack_index, contextualizer->generated_byte);

        strcpy(value, "");
        snprintf(value, strlen(value), "%s%s", HEXA_PREFIX, substring(token->lexeme, 3, 5));
        contextualizer->generated_byte = (int)strtol(value, NULL, 0);
        contextualizer->stack_index++;
        resize(&contextualizer->stack, contextualizer->stack_index + 1);
        push_at(&contextualizer->stack, contextualizer->stack_index, contextualizer->generated_byte);
        break;

    case LABEL_TRANSLATE:
        to_upper_case(token->lexeme);
        hashMapList_put(&(contextualizer->labels_buffer), token->lexeme, contextualizer->stack_index + 1);
        contextualizer->stack_index = contextualizer->stack_index + 2;
        resize(&contextualizer->stack, contextualizer->stack_index + 1);
        break;

    case LABEL_RECORD:
        to_upper_case(token->lexeme);
        char *label = substring(token->lexeme, 0, strlen(token->lexeme) - 1);

        if (hashMap_get(&(contextualizer->labels), label) == -1)
        {
            hashMap_put(&(contextualizer->labels), label, contextualizer->stack_index + 2);
        }
        else
        {
            int length = snprintf(NULL, 0, "Erro na posição %d: label '%s' duplicada.", token->position, label) + 1;
            contextualizer->error = (char *)malloc(length * sizeof(char));

            sprintf(contextualizer->error, "Erro na posição %d: label '%s' duplicada.", token->position, label);
        }
        free(label);
        break;

    default:
        break;
    }

    free(value);
}
