#include "memory.c"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>

#define REGISTERS 5
#define IO_SIZE 4
#define CALL_STACK_SIZE 4
#define A_ASCII 65
#define MAX_INT 256
#define MIN_INT 0
#define MIN_OUT_ADDR 20
#define MIN_IN_ADDR 24

typedef struct LabelNode
{
    char *label;
    int memPos;
    struct LabelNode *next;
} LabelNode;

typedef struct
{
    Memory memory;
    Stack call_stack;
    Stack data_stack;
    LabelNode *labelNode_head;
    int registers[REGISTERS];
    int inputs[IO_SIZE];
    int outputs[IO_SIZE];
    bool flag_c;
    bool flag_z;
    int timer;
    double delay;
    char *error;
} Machine;

void machine_init(Machine *machine)
{
    for (int i = 0; i < REGISTERS; i++)
    {
        machine->registers[i] = 0;
    }

    for (int i = 0; i < IO_SIZE; i++)
    {
        machine->inputs[i] = 0;
        machine->outputs[i] = 0;
    }

    memory_init(&(machine->memory));
    stack_init(&(machine->call_stack));
    stack_init(&(machine->data_stack));
    machine->labelNode_head = NULL;
    machine->flag_c = false;
    machine->flag_z = false;
    machine->error = (char *)malloc(192 * sizeof(char));

    strcpy(machine->error, "");
}

void set_timer(Machine *machine, int timer)
{
    machine->timer = timer;
}

void set_delay(Machine *machine, double delay)
{
    machine->delay = delay;
}

void labelNodes_free(LabelNode *head)
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

void machine_free(Machine *machine)
{
    stack_free(&(machine->call_stack));
    stack_free(&(machine->data_stack));
    labelNodes_free(machine->labelNode_head);
    memory_free(&(machine->memory));

    free(machine->error);

    machine->error = NULL;
    machine = NULL;
}

void delay(Machine *machine)
{
    clock_t start_time = clock();
    clock_t delay = machine->delay * CLOCKS_PER_SEC;
    while (clock() < start_time + delay)
    {
    }
}

TokenNode *get_label(Token *token, Machine *machine)
{
    LabelNode *labelNode = machine->labelNode_head;

    while (labelNode != NULL)
    {
        if (strncmp(labelNode->label, token->lexeme, strlen(token->lexeme)) == 0)
        {
            return get_token_at(labelNode->memPos, &(machine->memory));
        }

        labelNode = labelNode->next;
    }

    return NULL;
}

void add_label(Token *token, Machine *machine)
{
    LabelNode *new_labelNode = (LabelNode *)malloc(sizeof(LabelNode));
    new_labelNode->label = (char *)malloc(16 * sizeof(char));
    strcpy(new_labelNode->label, token->lexeme);
    new_labelNode->memPos = machine->memory.position - 1;
    new_labelNode->next = NULL;

    if (machine->labelNode_head == NULL)
    {
        machine->labelNode_head = new_labelNode;
    }
    else
    {
        LabelNode *current = machine->labelNode_head;
        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = new_labelNode;
    }
}

void check_flag_c(Machine *machine, int value)
{
    machine->flag_c = (value < MIN_INT || value > MAX_INT);
}

void check_flag_z(Machine *machine, int value)
{
    machine->flag_z = (value == MIN_INT);
}

int check_value(int number)
{
    if (number > MAX_INT)
    {
        return number -= MAX_INT;
    }

    if (number < MIN_INT)
    {
        return number += MAX_INT;
    }

    return number;
}

void and_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = number & machine->registers[register_to_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, machine->registers[register_to_position]);
    check_flag_z(machine, machine->registers[register_to_position]);
}

void and_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[register_to_position] & machine->registers[register_from_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, machine->registers[register_to_position]);
    check_flag_z(machine, machine->registers[register_to_position]);
}

void or_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = number | machine->registers[register_to_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, machine->registers[register_to_position]);
    check_flag_z(machine, machine->registers[register_to_position]);
}

void or_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[register_to_position] | machine->registers[register_from_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, machine->registers[register_to_position]);
    check_flag_z(machine, machine->registers[register_to_position]);
}

void xor_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = number ^ machine->registers[register_to_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, machine->registers[register_to_position]);
    check_flag_z(machine, machine->registers[register_to_position]);
}

void xor_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[register_to_position] ^ machine->registers[register_from_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, machine->registers[register_to_position]);
    check_flag_z(machine, machine->registers[register_to_position]);
}

void not_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = ~number;
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, machine->registers[register_to_position]);
    check_flag_z(machine, machine->registers[register_to_position]);
}

void not_register(char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = ~machine->registers[register_to_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, machine->registers[register_to_position]);
    check_flag_z(machine, machine->registers[register_to_position]);
}

void add_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[0] + number;
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, value);
    check_flag_z(machine, value);
}

void add_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[0] + machine->registers[register_from_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, value);
    check_flag_z(machine, value);
}

void sub_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[0] - number;
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, value);
    check_flag_z(machine, value);
}

void sub_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[0] - machine->registers[register_from_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, value);
    check_flag_z(machine, value);
}

void inc(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[register_from_position] + 1;
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine, value);
    check_flag_z(machine, value);
}

void mov_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    machine->registers[register_to_position] = check_value(number);

    check_flag_c(machine, number);
    check_flag_z(machine, number);
}

void mov_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    machine->registers[register_to_position] = check_value(machine->registers[register_from_position]);

    check_flag_c(machine, machine->registers[register_from_position]);
    check_flag_z(machine, machine->registers[register_from_position]);
}

void mov_output(char *register_from, char *output, Machine *machine)
{
    int output_addr;
    char *command = "i2c set -b";
    char buffer[100];

    sscanf(output, "OUT%d", &output_addr);
    int register_from_position = register_from[0] - A_ASCII;

    sprintf(buffer, "%s %d -a 0x%d 0x%X", command, CONFIG_EXAMPLES_M3P_I2C_BUS, (MIN_OUT_ADDR + output_addr), machine->outputs[output_addr]);

    if (system(buffer) == 0)
    {
        machine->outputs[output_addr] = machine->registers[register_from_position];
    }
    else
    {
        printf("Não foi possível ler a porta OUT%d.\n", output_addr);
    }
}

void mov_input(char *input, char *register_to, Machine *machine)
{
    int input_addr, value;
    FILE *file;
    char *command = "i2c get -b 0 -a";
    char buffer[100], response[100];

    sscanf(input, "IN%d", &input_addr);
    int register_to_position = register_to[0] - A_ASCII;

    sprintf(buffer, "%s 0x%d", command, (MIN_IN_ADDR + input_addr));

    file = popen(buffer, "r");
    if (file == NULL)
    {
        printf("Não foi possível ler a porta IN%d.\n", input_addr);
    }
    else
    {
        while (fgets(response, sizeof(response), file) != NULL)
        {
            if (sscanf(response, "READ %*s %*s %*s %*s %*s %*s Value: %d", &value) == 1)
            {
                machine->registers[register_to_position] = machine->inputs[input_addr] = (int)value;
            }
            else
            {
                printf("Não foi possível ler a porta IN%d.\n", input_addr);
            }
        }
        pclose(file);
    }
}

HttpResponse *execute(Machine *machine)
{
    time_t start_time = time(NULL);
    TokenNode *current = machine->memory.head;
    char *register_from, *register_to, *input, *output;
    int number, position;

    while (current != NULL)
    {
        switch (current->token->type)
        {
        case 14: // ADD
            current = current->next;
            if (current->token->type == 7) // NUMBER
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = current->next;
                register_to = current->token->lexeme;

                add_number(number, register_to, machine);
            }
            else // REGISTER
            {
                register_from = current->token->lexeme;

                current = current->next;
                register_to = current->token->lexeme;

                add_register(register_from, register_to, machine);
            }
            break;

        case 15: // SUB
            current = current->next;
            if (current->token->type == 7) // NUMBER
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = current->next;
                register_to = current->token->lexeme;

                sub_number(number, register_to, machine);
            }
            else // REGISTER
            {
                register_from = current->token->lexeme;

                current = current->next;
                register_to = current->token->lexeme;

                sub_register(register_from, register_to, machine);
            }
            break;

        case 16: // AND
            current = current->next;
            if (current->token->type == 7) // NUMBER
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = current->next;
                register_to = current->token->lexeme;

                and_number(number, register_to, machine);
            }
            else // REGISTER
            {
                register_from = current->token->lexeme;

                current = current->next;
                register_to = current->token->lexeme;

                and_register(register_from, register_to, machine);
            }
            break;

        case 17: // OR
            current = current->next;
            if (current->token->type == 7) // NUMBER
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = current->next;
                register_to = current->token->lexeme;

                or_number(number, register_to, machine);
            }
            else // REGISTER
            {
                register_from = current->token->lexeme;

                current = current->next;
                register_to = current->token->lexeme;

                or_register(register_from, register_to, machine);
            }
            break;

        case 18: // XOR
            current = current->next;
            if (current->token->type == 7) // NUMBER
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = current->next;
                register_to = current->token->lexeme;

                xor_number(number, register_to, machine);
            }
            else // REGISTER
            {
                register_from = current->token->lexeme;

                current = current->next;
                register_to = current->token->lexeme;

                xor_register(register_from, register_to, machine);
            }
            break;

        case 19: // NOT
            current = current->next;
            if (current->token->type == 7) // NUMBER
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = current->next;
                register_to = current->token->lexeme;

                not_number(number, register_to, machine);
            }
            else // REGISTER
            {
                register_to = current->token->lexeme;

                not_register(register_to, machine);
            }
            break;

        case 20: // MOV
            current = current->next;
            if (current->token->type == 7) // NUMBER
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = current->next;
                register_to = current->token->lexeme;

                mov_number(number, register_to, machine);
            }
            else if (current->token->type == 11) // INPUT
            {
                input = current->token->lexeme;

                current = current->next;
                register_to = current->token->lexeme;

                mov_input(input, register_to, machine);
            }
            else // REGISTER
            {
                register_from = current->token->lexeme;
                current = current->next;

                if (current->token->type == 12) // OUTPUT
                {
                    output = current->token->lexeme;

                    mov_output(register_from, output, machine);
                }
                else // REGISTER
                {
                    register_to = current->token->lexeme;

                    mov_register(register_from, register_to, machine);
                }
            }
            break;

        case 21: // INC
            current = current->next;

            register_from = current->token->lexeme;

            current = current->next;
            register_to = current->token->lexeme;

            inc(register_from, register_to, machine);
            break;

        case 22: // JMP
            current = current->next;
            current = get_label(current->token, machine);
            break;

        case 23: // JMPC
            current = current->next;

            if (machine->flag_c == true)
            {
                current = get_label(current->token, machine);
            }
            break;

        case 24: // JMPZ
            current = current->next;

            if (machine->flag_z == true)
            {
                current = get_label(current->token, machine);
            }
            break;

        case 25: // CALL
            if (!is_full(&(machine->call_stack)))
            {
                current = current->next;
                position = current->position;

                push(&(machine->call_stack), position);
                current = get_label(current->token, machine);
            }
            else
            {
                strcpy(machine->error, "StackOverflow: Pilha de chamadas sobrecarregada.");
            }
            break;

        case 26: // RET
            if (!is_empty(&(machine->call_stack)))
            {
                current = get_token_at(pop(&(machine->call_stack)), &(machine->memory));
            }
            else
            {
                strcpy(machine->error, "StackUnderflow: Pilha de chamadas vazia.");
            }
            break;

        case 27: // PUSH
            if (!is_full(&(machine->data_stack)))
            {
                current = current->next;
                number = strtol(current->token->lexeme, NULL, 16);

                push(&(machine->data_stack), number);
            }
            else
            {
                strcpy(machine->error, "StackOverflow: Pilha de dados sobrecarregada.");
            }
            break;

        case 28: // POP
            if (!is_empty(&(machine->data_stack)))
            {
                number = pop(&(machine->data_stack));

                current = current->next;
                register_to = current->token->lexeme;

                mov_number(number, register_to, machine);
            }
            else
            {
                strcpy(machine->error, "StackUnderflow: Pilha de dados vazia.");
                break;
            }
            break;

        default:
            break;
        }

        // interrompe a execucao se identificar algum erro
        if (strlen(machine->error) != 0)
        {
            break;
        }

        // interrompe a execucao se current for NULL
        if (current == NULL)
        {
            strcpy(machine->error, "Execução interrompida devido a erro inesperado.");
            break;
        }

        // interrompe a execucao se o tempo limite exceder
        time_t current_time = time(NULL);
        if ((current_time - start_time) >= machine->timer)
        {
            strcpy(machine->error, "Tempo limite atingido. Programa encerrado.");
            break;
        }

        current = current->next;

        delay(machine);
    }

    if (strlen(machine->error) != 0)
    {
        return httpResponse_create(machine->error, 400, "Bad Request");
    }

    return httpResponse_create("Execução concluída com suscesso.", 200, "OK");
}
