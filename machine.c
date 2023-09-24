#include "memory.c"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#define REGS_SIZE 5
#define IO_SIZE 4
#define CALL_STACK_SIZE 4
#define A_ASCII 65
#define MAX_INT 256
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
    int inputs[IO_SIZE];
    int outputs[IO_SIZE];
    bool flagC;
    bool flagZ;
    int timer;
    char *machineError;
    char *errorState;
    int errorCode;
} Machine;

void initializeMachine(Machine *machine)
{
    for (int i = 0; i < REGS_SIZE; i++)
    {
        machine->regs[i] = 0;
    }

    for (int i = 0; i < IO_SIZE; i++)
    {
        machine->inputs[i] = 0;
        machine->outputs[i] = 0;
    }

    initializeMemory(&(machine->memory));
    initializeStack(&(machine->callStack));
    initializeStack(&(machine->dataStack));
    machine->labelHead = NULL;
    machine->flagC = false;
    machine->flagZ = false;
    machine->machineError = (char *)malloc(192 * sizeof(char));
    machine->errorState = (char *)malloc(32 * sizeof(char));

    strcpy(machine->machineError, "");
    strcpy(machine->errorState, "");
}

void setInputs(Machine *machine, char *in0, char *in1, char *in2, char *in3)
{
    machine->inputs[0] = strtol(in0, NULL, 16);
    machine->inputs[1] = strtol(in1, NULL, 16);
    machine->inputs[2] = strtol(in2, NULL, 16);
    machine->inputs[3] = strtol(in3, NULL, 16);
}

void setTimer(Machine *machine, int timer)
{
    machine->timer = timer;
}

void freeLabelNodes(LabelNode *head)
{
    LabelNode *current = head;
    LabelNode *next = NULL;

    while (current != NULL)
    {
        next = current->next;
        free(current->label);
        free(current);
        current = next;
    }
}

void freeMachine(Machine *machine)
{
    clear(&(machine->callStack));
    clear(&(machine->dataStack));
    freeLabelNodes(machine->labelHead);
    freeMemory(&(machine->memory));

    free(machine->machineError);
    free(machine->errorState);
    
    machine->machineError = NULL;
    machine->errorState = NULL;
    machine = NULL;
}

void addOUTsToMessage(Machine *machine){
    char value[10];
    strcat(machine->machineError, "\nOUT0:");
    sprintf(value, "%X", machine->outputs[0]);
    strcat(machine->machineError, value);

    strcat(machine->machineError, " OUT1:");
    sprintf(value, "%X", machine->outputs[1]);
    strcat(machine->machineError, value);

    strcat(machine->machineError, " OUT2:");
    sprintf(value, "%X", machine->outputs[2]);
    strcat(machine->machineError, value);

    strcat(machine->machineError, " OUT3:");
    sprintf(value, "%X", machine->outputs[3]);
    strcat(machine->machineError, value);
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

    return num;
}

void andN(int num, char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    int value = num & machine->regs[regPos];
    machine->regs[regPos] = checkNumValue(value);

    checkFlagC(machine, machine->regs[regPos]);
    checkFlagZ(machine, machine->regs[regPos]);
}

void andR(char *from, char *to, Machine *machine)
{
    int regFromPos = from[0] - A_ASCII;
    int regToPos = to[0] - A_ASCII;
    int value = machine->regs[regToPos] & machine->regs[regFromPos];
    machine->regs[regToPos] = checkNumValue(value);

    checkFlagC(machine, machine->regs[regToPos]);
    checkFlagZ(machine, machine->regs[regToPos]);
}

void orN(int num, char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    int value = num | machine->regs[regPos];
    machine->regs[regPos] = checkNumValue(value);

    checkFlagC(machine, machine->regs[regPos]);
    checkFlagZ(machine, machine->regs[regPos]);
}

void orR(char *from, char *to, Machine *machine)
{
    int regFromPos = from[0] - A_ASCII;
    int regToPos = to[0] - A_ASCII;
    int value = machine->regs[regToPos] | machine->regs[regFromPos];
    machine->regs[regToPos] = checkNumValue(value);

    checkFlagC(machine, machine->regs[regToPos]);
    checkFlagZ(machine, machine->regs[regToPos]);
}

void xorN(int num, char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    int value = num ^ machine->regs[regPos];
    machine->regs[regPos] = checkNumValue(value);

    checkFlagC(machine, machine->regs[regPos]);
    checkFlagZ(machine, machine->regs[regPos]);
}

void xorR(char *from, char *to, Machine *machine)
{
    int regFromPos = from[0] - A_ASCII;
    int regToPos = to[0] - A_ASCII;
    int value = machine->regs[regToPos] ^ machine->regs[regFromPos];
    machine->regs[regToPos] = checkNumValue(value);

    checkFlagC(machine, machine->regs[regToPos]);
    checkFlagZ(machine, machine->regs[regToPos]);
}

void notN(int num, char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    int value = ~num;
    machine->regs[regPos] = checkNumValue(value);

    checkFlagC(machine, machine->regs[regPos]);
    checkFlagZ(machine, machine->regs[regPos]);
}

void notR(char *reg, Machine *machine)
{
    int regPos = reg[0] - A_ASCII;
    int value = ~machine->regs[regPos];
    machine->regs[regPos] = checkNumValue(value);

    checkFlagC(machine, machine->regs[regPos]);
    checkFlagZ(machine, machine->regs[regPos]);
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
    int value = machine->regs[regFromPos] + 1;
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

void movO(char *from, char *out, Machine *machine)
{
    int outPos;
    sscanf(out, "OUT%d", &outPos);
    int regFromPos = from[0] - A_ASCII;
     machine->outputs[outPos] = machine->regs[regFromPos];

    printf("%s: %X\n", out, machine->regs[regFromPos]);
}

void movI(char *in, char *to, Machine *machine)
{
    int inPos;
    sscanf(in, "IN%d", &inPos);
    int regToPos = to[0] - A_ASCII;

    machine->regs[regToPos] = machine->inputs[inPos];
}

HttpResponse *execute(Machine *machine)
{
    time_t startTime = time(NULL);
    TokenNode *current = machine->memory.head;

    while (current != NULL)
    {
        switch (current->token->type)
        {
        case 14: // ADD
            current = current->next;
            if (current->token->type == 7) // NUM
            {
                int num = strtol(current->token->lexeme, NULL, 16);
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
                int num = strtol(current->token->lexeme, NULL, 16);
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
                int num = strtol(current->token->lexeme, NULL, 16);
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
                int num = strtol(current->token->lexeme, NULL, 16);
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
                int num = strtol(current->token->lexeme, NULL, 16);
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
            if (current->token->type == 7) // NUM
            {
                int num = strtol(current->token->lexeme, NULL, 16);
                current = current->next;
                notN(num, current->token->lexeme, machine);
            }
            else // REG
            {
                notR(current->token->lexeme, machine);
            }
            break;

        case 20: // MOV
            current = current->next;
            if (current->token->type == 7) // NUM
            {
                int num = strtol(current->token->lexeme, NULL, 16);
                current = current->next;
                movN(num, current->token->lexeme, machine);
            }
            else if (current->token->type == 11) // IN
            {
                char *in = current->token->lexeme;
                current = current->next;

                movI(in, current->token->lexeme, machine);
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
            }
            break;

        case 24: // JMPZ
            current = current->next;

            if (machine->flagZ == true)
            {
                current = getLabel(current->token, machine);
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
                strcpy(machine->machineError, "StackOverflow: Pilha de chamadas sobrecarregada");
                strcpy(machine->errorState, "Bad Request");
                machine->errorCode = 400;
                break;
            }
            break;

        case 26: // RET
            if (!isEmpty(&(machine->callStack)))
            {
                current = getTokenAt(pop(&(machine->callStack)), &(machine->memory));
            }
            else
            {
                strcpy(machine->machineError, "StackUnderflow: Pilha de chamadas vazia");
                strcpy(machine->errorState, "Bad Request");
                machine->errorCode = 400;
                break;
            }
            break;

        case 27: // PUSH
            if (!isFull(&(machine->dataStack)))
            {
                int num = strtol(current->token->lexeme, NULL, 16);
                push(&(machine->dataStack), num);
            }
            else
            {
                strcpy(machine->machineError, "StackOverflow: Pilha de dados sobrecarregada");
                strcpy(machine->errorState, "Bad Request");
                machine->errorCode = 400;
                break;
            }
            break;

        case 28: // POP

            if (!isEmpty(&(machine->dataStack)))
            {
                pop(&(machine->dataStack));
            }
            else
            {
                strcpy(machine->machineError, "StackUnderflow: Pilha de dados vazia");
                strcpy(machine->errorState, "Bad Request");
                machine->errorCode = 400;
                break;
            }
            break;

        default:
            break;
        }

        // interrompe a execucao se identificar algum erro
        if (strlen(machine->machineError) != 0)
        {
            break;
        }

        // interrompe a execucao se o tempo limite exceder
        time_t currentTime = time(NULL);
        if ((currentTime - startTime) >= machine->timer)
        {
            strcpy(machine->machineError, "Tempo limite atingido. Encerrando o programa");
            strcpy(machine->errorState, "OK");
            machine->errorCode = 200;
            break;
        }

        // interrompe a execucao se current for NULL
        if (current == NULL)
        {
            strcpy(machine->machineError, "Execucao interrompida devido a erro inesperado");
            strcpy(machine->errorState, "Internal Server Error");
            machine->errorCode = 500;
            break;
        }

        current = current->next;
    }

    addOUTsToMessage(machine);
    if (strlen(machine->machineError) != 0)
    {
        return createHttpResponse(machine->machineError, machine->errorCode, machine->errorState);
    }

    return createHttpResponse("Execucao concluida com suscesso", 200, "OK");
}
