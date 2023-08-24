#include "memory.c"
#include <string.h>
#include <stdlib.h>
#define REGS_SIZE 5
#define CALL_STACK_SIZE 4
#define A_ASCII 65
#define MAX_INT 255
#define MIN_INT 0

typedef struct LabelNode
{
    char *label;
    int memPos;
    struct LabelNode *next;
} LabelNode;

typedef struct
{
    Memory memory;
    Stack callStack;
    Stack dataStack;
    LabelNode *labelHead;
    int regs[REGS_SIZE];
    int pos;
    bool flagC;
    bool flagZ;
} Machine;

void initializeMachine(Machine *machine)
{
    for (int i = 0; i < REGS_SIZE; i++)
    {
        machine->regs[i] = 0;
    }

    initializeMemory(&(machine->memory));
    initializeStack(&(machine->callStack));
    initializeStack(&(machine->dataStack));
    machine->labelHead = NULL;
    machine->pos = 0;
    machine->flagC = false;
    machine->flagZ = false;
}

void freeLabelNodes(LabelNode *head)
{
    LabelNode *current = head;
    LabelNode *proximo = NULL;

    while (current != NULL)
    {
        proximo = current->next;
        free(current->label);
        free(current);
        current = proximo;
    }

    proximo = NULL;
    head = NULL;
}

void freeMachine(Machine *machine)
{
    clear(&(machine->callStack));
    clear(&(machine->dataStack));
    freeLabelNodes(machine->labelHead);
    freeMemory(&(machine->memory));

    machine = NULL;
}

TokenNode *getLabel(Token *token, Machine *machine)
{
    LabelNode *labelNode = machine->labelHead;

    while (labelNode != NULL)
    {
        if (strncmp(labelNode->label, token->lexeme, strlen(token->lexeme)) == 0)
        {
            return getTokenAt(labelNode->memPos, &(machine->memory));
        }

        labelNode = labelNode->next;
    }

    return NULL;
}

void addLabel(Token *token, Machine *machine)
{
    LabelNode *newNode = (LabelNode *)malloc(sizeof(LabelNode));
    newNode->label = (char *)malloc(16 * sizeof(char));
    strcpy(newNode->label, token->lexeme);
    newNode->memPos = machine->memory.pos - 1;
    newNode->next = NULL;

    if (machine->labelHead == NULL)
    {
        machine->labelHead = newNode;
    }
    else
    {
        LabelNode *current = machine->labelHead;
        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = newNode;
    }
}

void checkFlagC(Machine *machine, int value)
{
    machine->flagC = (value < MIN_INT || value > MAX_INT);
}

void checkFlagZ(Machine *machine, int value)
{
    machine->flagZ = (value == MIN_INT);
}

int checkNumValue(int num)
{
    if (num > MAX_INT)
    {
        return num -= MAX_INT;
    }

    if (num < MIN_INT)
    {
        return num += MAX_INT;
    }
}

void andN(int num, char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    int value = num & machine->regs[regPos];
    machine->regs[regPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void andR(char *from, char *to, Machine *machine)
{
    int regFromPos = from[0] - A_ASCII;
    int regToPos = to[0] - A_ASCII;
    int value = machine->regs[regToPos] & machine->regs[regFromPos];
    machine->regs[regToPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void orN(int num, char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    int value = num | machine->regs[regPos];
    machine->regs[regPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void orR(char *from, char *to, Machine *machine)
{
    int regFromPos = from[0] - A_ASCII;
    int regToPos = to[0] - A_ASCII;
    int value = machine->regs[regToPos] | machine->regs[regFromPos];
    machine->regs[regToPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void xorN(int num, char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    int value = num ^ machine->regs[regPos];
    machine->regs[regPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void xorR(char *from, char *to, Machine *machine)
{
    int regFromPos = from[0] - A_ASCII;
    int regToPos = to[0] - A_ASCII;
    int value = machine->regs[regToPos] ^ machine->regs[regFromPos];
    machine->regs[regToPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void not(char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    int value = ~machine->regs[regPos];
    machine->regs[regPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void addN(int num, char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    int value = machine->regs[0] + num;
    machine->regs[regPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void addR(char *from, char *to, Machine *machine)
{
    int regFromPos = from[0] - A_ASCII;
    int regToPos = to[0] - A_ASCII;
    int value = machine->regs[0] + machine->regs[regFromPos];
    machine->regs[regToPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void subN(int num, char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    int value = machine->regs[0] - num;
    machine->regs[regPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void subR(char *from, char *to, Machine *machine)
{
    int regFromPos = from[0] - A_ASCII;
    int regToPos = to[0] - A_ASCII;
    int value = machine->regs[0] - machine->regs[regFromPos];
    machine->regs[regToPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void inc(char *from, char *to, Machine *machine)
{
    int regFromPos = from[0] - A_ASCII;
    int regToPos = to[0] - A_ASCII;
    int value = machine->regs[regFromPos] + (machine->regs[0] + 1);
    machine->regs[regToPos] = checkNumValue(value);

    checkFlagC(machine, value);
    checkFlagZ(machine, value);
}

void movN(int num, char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    machine->regs[regPos] = checkNumValue(num);

    checkFlagC(machine, num);
    checkFlagZ(machine, num);
}

void movR(char *from, char *to, Machine *machine)
{
    int regFromPos = from[0] - A_ASCII;
    int regToPos = to[0] - A_ASCII;
    machine->regs[regToPos] = checkNumValue(machine->regs[regFromPos]);

    checkFlagC(machine, machine->regs[regFromPos]);
    checkFlagZ(machine, machine->regs[regFromPos]);
}

// indetificar OUT (1, 2, 3) e exibir valor
void movO(char *from, char *out, Machine *machine)
{
    int regFromPos = from[0] - A_ASCII;

    printf("%s: %d\n", out, machine->regs[regFromPos]);
}

HttpResponse *execute(Machine *machine)
{
    TokenNode *current = machine->memory.head;

    while (current != NULL)
    {
        switch (current->token->type)
        {
        case 14: // ADD
            current = current->next;
            if (current->token->type == 7) // NUM
            {
                int num = atoi(current->token->lexeme);
                current = current->next;
                addN(num, current->token->lexeme, machine);
            }
            else // REG
            {
                char *from = current->token->lexeme;
                current = current->next;
                addR(from, current->token->lexeme, machine);
            }
            break;

        case 15: // SUB
            current = current->next;
            if (current->token->type == 7) // NUM
            {
                int num = atoi(current->token->lexeme);
                current = current->next;
                subN(num, current->token->lexeme, machine);
            }
            else // REG
            {
                char *from = current->token->lexeme;
                current = current->next;
                subR(from, current->token->lexeme, machine);
            }
            break;

        case 16: // AND
            current = current->next;
            if (current->token->type == 7) // NUM
            {
                int num = atoi(current->token->lexeme);
                current = current->next;
                andN(num, current->token->lexeme, machine);
            }
            else // REG
            {
                char *from = current->token->lexeme;
                current = current->next;
                andR(from, current->token->lexeme, machine);
            }
            break;

        case 17: // OR
            current = current->next;
            if (current->token->type == 7) // NUM
            {
                int num = atoi(current->token->lexeme);
                current = current->next;
                orN(num, current->token->lexeme, machine);
            }
            else // REG
            {
                char *from = current->token->lexeme;
                current = current->next;
                orR(from, current->token->lexeme, machine);
            }
            break;

        case 18: // XOR
            current = current->next;
            if (current->token->type == 7) // NUM
            {
                int num = atoi(current->token->lexeme);
                current = current->next;
                xorN(num, current->token->lexeme, machine);
            }
            else // REG
            {
                char *from = current->token->lexeme;
                current = current->next;
                xorR(from, current->token->lexeme, machine);
            }
            break;

        case 19: // NOT
            current = current->next;
            not(current->token->lexeme, machine);
            break;

        case 20: // MOV
            current = current->next;
            if (current->token->type == 7) // NUM
            {
                int num = atoi(current->token->lexeme);
                current = current->next;
                movN(num, current->token->lexeme, machine);
            }
            else // REG
            {
                char *from = current->token->lexeme;
                current = current->next;

                if (current->token->type == 12) // OUT
                {
                    movO(from, current->token->lexeme, machine);
                }
                else // REG
                {
                    movR(from, current->token->lexeme, machine);
                }
            }
            break;

        case 21: // INC
            current = current->next;

            char *from = current->token->lexeme;
            current = current->next;
            inc(from, current->token->lexeme, machine);
            break;

        case 22: // JMP
            current = current->next;
            current = getLabel(current->token, machine);
            break;

        case 23: // JMPC
            current = current->next;

            if (machine->flagC == true)
            {
                current = getLabel(current->token, machine);
                machine->flagC = false;
                machine->flagZ = false;
            }
            break;

        case 24: // JMPZ
            current = current->next;

            if (machine->flagZ == true)
            {
                current = getLabel(current->token, machine);
                machine->flagC = false;
                machine->flagZ = false;
            }
            break;

        case 25: // CALL
            if (!isFull(&(machine->callStack)))
            {
                current = current->next;
                push(&(machine->callStack), current->pos);
                current = getLabel(current->token, machine);
            }
            else
            {
                return createHttpResponse("StackOverflow: Pilha de chamadas sobrecarregada", 400, "Bad Request");
            }
            break;

        case 26: // RET
            if (!isEmpty(&(machine->callStack)))
            {
                current = getTokenAt(pop(&(machine->callStack)), &(machine->memory));
            }
            else
            {
                return createHttpResponse("StackUnderflow: Pilha de chamadas vazia", 400, "Bad Request");
            }
            break;

        case 27: // PUSH
            if (!isFull(&(machine->dataStack)))
            {
                int num = atoi(current->token->lexeme);
                push(&(machine->dataStack), num);
            }
            else
            {
                return createHttpResponse("StackOverflow: Pilha de dados sobrecarregada", 400, "Bad Request");
            }
            break;

        case 28: // POP

            if (!isEmpty(&(machine->dataStack)))
            {
                pop(&(machine->dataStack));
            }
            else
            {
                return createHttpResponse("StackUnderflow: Pilha de dados vazia", 400, "Bad Request");
            }
            break;

        default:
            break;
        }

        if (current == NULL)
        {
            return createHttpResponse("Execucao interrompida devido a erro inesperado", 500, "Internal Server Error");
        }

        current = current->next;
    }

    return createHttpResponse("Execucao concluida com suscesso", 200, "OK");
}
