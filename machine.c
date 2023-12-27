#include <time.h>
#include <stdio.h>

#define REGISTERS 5
#define A_ASCII 65
#define MAX_INT 256
#define MIN_INT 0
#define MIN_OUT_ADDR 20
#define MIN_IN_ADDR 24

typedef struct TokenNode
{
    Token *token;
    int position;
    bool is_label;
    struct TokenNode *next;
} TokenNode;

typedef struct
{
    Stack call_stack;
    Stack data_stack;
    TokenNode *tokenNode_head;
    int registers[REGISTERS];
    bool flag_c;
    bool flag_z;
    int timer;
    int tokens;
    double delay;
    char *error;
} Machine;

void machine_init(Machine *machine)
{
    for (int i = 0; i < REGISTERS; i++)
    {
        machine->registers[i] = 0;
    }

    stack_init(&(machine->call_stack));
    stack_init(&(machine->data_stack));

    machine->tokens = 0;
    machine->tokenNode_head = NULL;
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

void tokenNodes_free(TokenNode *head)
{
    TokenNode *current = head;
    TokenNode *next = NULL;

    while (current != NULL)
    {
        next = current->next;
        token_free(current->token);
        free(current);
        current = next;
    }
}

void machine_free(Machine *machine)
{
    stack_free(&(machine->call_stack));
    stack_free(&(machine->data_stack));
    tokenNodes_free(machine->tokenNode_head);

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

void add_token(Token *token, Machine *machine)
{
    if (token == NULL)
    {
        return;
    }

    TokenNode *new_tokenNode = (TokenNode *)malloc(sizeof(TokenNode));
    new_tokenNode->token = token;
    new_tokenNode->position = machine->tokens++;
    new_tokenNode->is_label = token->type == 29;
    new_tokenNode->next = NULL;

    if (machine->tokenNode_head == NULL)
    {
        machine->tokenNode_head = new_tokenNode;
    }
    else
    {
        TokenNode *current = machine->tokenNode_head;
        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = new_tokenNode;
    }
}

TokenNode *get_next_valid_token(TokenNode *current, Machine *machine)
{
    if (current == NULL)
    {
        return NULL;
    }

    TokenNode *tokenNode = current->next;
    int length = sizeof(MACHINE_SET_CASES_VALUES) / sizeof(MACHINE_SET_CASES_VALUES[0]);
    bool valid = false;

    while (tokenNode != NULL)
    {
        for (size_t i = 0; i < length; i++)
        {
            if (tokenNode->token->type == MACHINE_SET_CASES_VALUES[i])
            {
                valid = true;
                break;
            }
        }

        if (valid)
        {
            break;
        }

        tokenNode = tokenNode->next;
    }

    return tokenNode;
}

TokenNode *get_token_at(int position, Machine *machine)
{
    if (machine->tokenNode_head == NULL)
    {
        return NULL;
    }

    TokenNode *tokenNode = machine->tokenNode_head;
    while (tokenNode->position < position && tokenNode != NULL)
    {
        tokenNode = tokenNode->next;
    }

    return tokenNode;
}

TokenNode *get_token_by_label(Token *token, Machine *machine)
{
    TokenNode *tokenNode = machine->tokenNode_head;

    while (tokenNode != NULL)
    {
        if (tokenNode->is_label && strncmp(tokenNode->token->lexeme, token->lexeme, strlen(token->lexeme)) == 0)
        {
            return tokenNode;
        }

        tokenNode = tokenNode->next;
    }

    return NULL;
}

void check_flag_c(int value, Machine *machine)
{
    machine->flag_c = (value < MIN_INT || value > MAX_INT);
}

void check_flag_z(int value, Machine *machine)
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

    check_flag_c(machine->registers[register_to_position], machine);
    check_flag_z(machine->registers[register_to_position], machine);
}

void and_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[register_to_position] & machine->registers[register_from_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine->registers[register_to_position], machine);
    check_flag_z(machine->registers[register_to_position], machine);
}

void or_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = number | machine->registers[register_to_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine->registers[register_to_position], machine);
    check_flag_z(machine->registers[register_to_position], machine);
}

void or_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[register_to_position] | machine->registers[register_from_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine->registers[register_to_position], machine);
    check_flag_z(machine->registers[register_to_position], machine);
}

void xor_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = number ^ machine->registers[register_to_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine->registers[register_to_position], machine);
    check_flag_z(machine->registers[register_to_position], machine);
}

void xor_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[register_to_position] ^ machine->registers[register_from_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine->registers[register_to_position], machine);
    check_flag_z(machine->registers[register_to_position], machine);
}

void not_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = ~number;
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine->registers[register_to_position], machine);
    check_flag_z(machine->registers[register_to_position], machine);
}

void not_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = ~machine->registers[register_from_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(machine->registers[register_to_position], machine);
    check_flag_z(machine->registers[register_to_position], machine);
}

void add_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[0] + number;
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void add_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[0] + machine->registers[register_from_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void sub_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[0] - number;
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void sub_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[0] - machine->registers[register_from_position];
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void inc(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    int value = machine->registers[register_from_position] + 1;
    machine->registers[register_to_position] = check_value(value);

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void mov_number(int number, char *register_to, Machine *machine)
{
    int register_to_position = register_to[0] - A_ASCII;
    machine->registers[register_to_position] = check_value(number);

    check_flag_c(number, machine);
    check_flag_z(number, machine);
}

void mov_register(char *register_from, char *register_to, Machine *machine)
{
    int register_from_position = register_from[0] - A_ASCII;
    int register_to_position = register_to[0] - A_ASCII;
    machine->registers[register_to_position] = check_value(machine->registers[register_from_position]);

    check_flag_c(machine->registers[register_from_position], machine);
    check_flag_z(machine->registers[register_from_position], machine);
}

void mov_output(char *register_from, char *output, Machine *machine)
{
    int output_addr;
    char *command = "i2c set -b";
    char buffer[100];

    sscanf(output, "OUT%d", &output_addr);
    int register_from_position = register_from[0] - A_ASCII;

    sprintf(buffer, "%s %d -a 0x%d 0x%X", command, CONFIG_EXAMPLES_M3P_I2C_BUS, (MIN_OUT_ADDR + output_addr), machine->registers[register_from_position]);

    if (system(buffer) != 0)
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
                machine->registers[register_to_position] = (int)value;
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
    char *register_from, *register_to, *input, *output;
    int number, position;
    TokenNode *current = machine->tokenNode_head;
    while (current != NULL)
    {
        switch (current->token->type)
        {
        case INSTRUCTION_ADD:
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução ADD.");
                break;
            }

            if (current->token->type == NUMBER)
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução ADD.");
                    break;
                }

                register_to = current->token->lexeme;

                add_number(number, register_to, machine);
            }
            else if (current->token->type >= REGISTER_A && current->token->type <= REGISTER_E)
            {
                register_from = current->token->lexeme;

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução ADD.");
                    break;
                }

                register_to = current->token->lexeme;

                add_register(register_from, register_to, machine);
            }
            else
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução ADD.");
            }
            break;

        case INSTRUCTION_SUB:
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução SUB.");
                break;
            }

            if (current->token->type == NUMBER)
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução SUB.");
                    break;
                }

                register_to = current->token->lexeme;

                sub_number(number, register_to, machine);
            }
            else if (current->token->type >= REGISTER_A && current->token->type <= REGISTER_E)
            {
                register_from = current->token->lexeme;

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução SUB.");
                    break;
                }

                register_to = current->token->lexeme;

                sub_register(register_from, register_to, machine);
            }
            else
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução SUB.");
            }
            break;

        case INSTRUCTION_AND:
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução AND.");
                break;
            }

            if (current->token->type == NUMBER)
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução AND.");
                    break;
                }

                register_to = current->token->lexeme;

                and_number(number, register_to, machine);
            }
            else if (current->token->type >= REGISTER_A && current->token->type <= REGISTER_E)
            {
                register_from = current->token->lexeme;

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução AND.");
                    break;
                }

                register_to = current->token->lexeme;

                and_register(register_from, register_to, machine);
            }
            else
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução AND.");
            }
            break;

        case INSTRUCTION_OR:
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução OR.");
                break;
            }

            if (current->token->type == NUMBER)
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução OR.");
                    break;
                }

                register_to = current->token->lexeme;

                or_number(number, register_to, machine);
            }
            else if (current->token->type >= REGISTER_A && current->token->type <= REGISTER_E)
            {
                register_from = current->token->lexeme;

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução OR.");
                    break;
                }

                register_to = current->token->lexeme;

                or_register(register_from, register_to, machine);
            }
            else
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução OR.");
            }
            break;

        case INSTRUCTION_XOR:
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução XOR.");
                break;
            }

            if (current->token->type == NUMBER)
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução XOR.");
                    break;
                }

                register_to = current->token->lexeme;

                xor_number(number, register_to, machine);
            }
            else if (current->token->type >= REGISTER_A && current->token->type <= REGISTER_E)
            {
                register_from = current->token->lexeme;

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução XOR.");
                    break;
                }

                register_to = current->token->lexeme;

                xor_register(register_from, register_to, machine);
            }
            else
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução XOR.");
            }
            break;

        case INSTRUCTION_NOT:
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução NOT.");
                break;
            }

            if (current->token->type == NUMBER)
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução NOT.");
                    break;
                }

                register_to = current->token->lexeme;

                not_number(number, register_to, machine);
            }
            else if (current->token->type >= REGISTER_A && current->token->type <= REGISTER_E)
            {
                register_from = current->token->lexeme;

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução NOT.");
                    break;
                }

                register_to = current->token->lexeme;

                not_register(register_from, register_to, machine);
            }
            else
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução NOT.");
            }
            break;

        case INSTRUCTION_MOV:
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução MOV.");
                break;
            }

            if (current->token->type == NUMBER)
            {
                number = strtol(current->token->lexeme, NULL, 16);

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução MOV.");
                    break;
                }

                register_to = current->token->lexeme;

                mov_number(number, register_to, machine);
            }
            else if (current->token->type == INPUT_PORT)
            {
                input = current->token->lexeme;

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução MOV.");
                    break;
                }

                register_to = current->token->lexeme;

                mov_input(input, register_to, machine);
            }
            else if (current->token->type >= REGISTER_A && current->token->type <= REGISTER_E)
            {
                register_from = current->token->lexeme;
                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução MOV.");
                    break;
                }

                if (current->token->type == OUTPUT_PORT)
                {
                    output = current->token->lexeme;

                    mov_output(register_from, output, machine);
                }
                else if (current->token->type >= REGISTER_A && current->token->type <= REGISTER_E)
                {
                    register_to = current->token->lexeme;

                    mov_register(register_from, register_to, machine);
                }
                else
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução MOV.");
                }
            }
            else
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução MOV.");
            }
            break;

        case INSTRUCTION_INC:
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução INC.");
                break;
            }

            if (current->token->type >= REGISTER_A && current->token->type <= REGISTER_E)
            {
                register_from = current->token->lexeme;

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução INC.");
                    break;
                }

                if (current->token->type >= REGISTER_A && current->token->type <= REGISTER_E)
                {
                    register_to = current->token->lexeme;

                    inc(register_from, register_to, machine);
                }
                else
                {
                    strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução INC.");
                }
            }
            else
            {
                strcpy(machine->error, "InvalidArgumentException: Parâmetros inválidos para a instrução INC.");
            }
            break;

        case INSTRUCTION_JMP:
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                strcpy(machine->error, "InvalidArgumentException: parâmetro inválido para a instrução JMP.");
                break;
            }

            current = get_token_by_label(current->token, machine);
            break;

        case INSTRUCTION_JMPC:
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                strcpy(machine->error, "InvalidArgumentException: parâmetro inválido para a instrução JMPC.");
                break;
            }

            if (machine->flag_c == true)
            {
                current = get_token_by_label(current->token, machine);
            }
            break;

        case INSTRUCTION_JMPZ:
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                strcpy(machine->error, "InvalidArgumentException: parâmetro inválido para a instrução JMPZ.");
                break;
            }

            if (machine->flag_z == true)
            {
                current = get_token_by_label(current->token, machine);
            }
            break;

        case INSTRUCTION_CALL:
            if (!is_full(&(machine->call_stack)))
            {
                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: parâmetro inválido para a instrução CALL.");
                    break;
                }

                position = current->position;

                push(&(machine->call_stack), position);
                current = get_token_by_label(current->token, machine);
            }
            else
            {
                strcpy(machine->error, "StackOverflow: Pilha de chamadas sobrecarregada.");
            }
            break;

        case INSTRUCTION_RET:
            if (!is_empty(&(machine->call_stack)))
            {
                current = get_token_at(pop(&(machine->call_stack)), machine);
            }
            else
            {
                strcpy(machine->error, "StackUnderflow: Pilha de chamadas vazia.");
            }
            break;

        case INSTRUCTION_PUSH:
            if (!is_full(&(machine->data_stack)))
            {
                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: parâmetro inválido para a instrução PUSH.");
                    break;
                }

                number = strtol(current->token->lexeme, NULL, 16);

                push(&(machine->data_stack), number);
            }
            else
            {
                strcpy(machine->error, "StackOverflow: Pilha de dados sobrecarregada.");
            }
            break;

        case INSTRUCTION_POP:
            if (!is_empty(&(machine->data_stack)))
            {
                number = pop(&(machine->data_stack));

                current = get_next_valid_token(current, machine);
                if (current == NULL)
                {
                    strcpy(machine->error, "InvalidArgumentException: parâmetro inválido para a instrução POP.");
                    break;
                }

                register_to = current->token->lexeme;

                mov_number(number, register_to, machine);
            }
            else
            {
                strcpy(machine->error, "StackUnderflow: Pilha de dados vazia.");
                break;
            }
            break;

        default: // entrará aqui em caso de virgula, dois pontos, label ou qualquer outro token não mapeado

            if (current == NULL)
            {
                strcpy(machine->error, "NullPointerException: Nenhum token encontrado para interpretar.");
                break;
            }

            if (current->token->type != LABEL_ID && current->token->type != LABEL &&
                current->token->type != SEMICOLON && current->token->type != COMA)
            {
                strcpy(machine->error, "InvalidArgumentException: Token não mapeado.");
            }
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

        current = current->next; // retorna o próximo token independente do tipo

        delay(machine);
    }

    if (strlen(machine->error) != 0)
    {
        return httpResponse_create(machine->error, 400, "Bad Request");
    }

    return httpResponse_create("Execução concluída com suscesso.", 200, "OK");
}
