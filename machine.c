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
    List memory;
    int registers[REGISTERS];
    bool flag_c;
    bool flag_z;
    int timer;
    int tokens;
    double delay;
    char *message;
} Machine;

void machine_init(Machine *machine)
{
    for (int i = 0; i < REGISTERS; i++)
    {
        machine->registers[i] = 0;
    }

    stack_init(&(machine->call_stack));
    stack_init(&(machine->data_stack));
    list_init(MAX_INT, &(machine->memory));

    machine->tokens = 0;
    machine->tokenNode_head = NULL;
    machine->message = NULL;
    machine->flag_c = false;
    machine->flag_z = false;
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
    list_free(&(machine->memory));

    if (machine->message != NULL)
    {
        free(machine->message);
    }
    machine->message = NULL;
    machine = NULL;
}

void set_timer(Machine *machine, int timer)
{
    machine->timer = timer;
}

void set_delay(Machine *machine, double delay)
{
    machine->delay = delay;
}

void set_machine_response(char *message, Machine *machine)
{
    machine->message = (char *)malloc((strlen(message) + 1) * sizeof(char));
    sprintf(machine->message, message);
}

void delay(double time)
{
    clock_t start_time = clock();
    clock_t delay = time * CLOCKS_PER_SEC;
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

void add(TokenNode *tokenNode1, TokenNode *tokenNode2, Machine *machine)
{
    if (tokenNode1 == NULL || tokenNode2 == NULL) // validação para garantir funcionamento
    {
        set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução ADD.", machine);
        return;
    }

    int number = 0;

    if (tokenNode1->token->type == HEXADECIMAL && (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // ADD HEX,REG
    {
        number = strtol(tokenNode1->token->lexeme, NULL, 16); // obtem o valor decimal através do hex fornecido
    }
    else if ((tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E) &&
             (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // ADD REG,REG
    {
        int register_from = tokenNode1->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de origem no vetor
        number = machine->registers[register_from];                 // obtem o valor no registrador de origem
    }
    else // sintaxe incorreta
    {
        set_machine_response("InvalidArgumentException: Parâmetros inválidos para a instrução ADD.", machine);
        return;
    }

    int register_to = tokenNode2->token->lexeme[0] - A_ASCII; // obtem a posição do registrador no vetor
    int value = machine->registers[0] + number;               // valor do registrador A + valor do registrador de origem
    machine->registers[register_to] = check_value(value);     // atribui o valor corrigido ao registrador de destino

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void sub(TokenNode *tokenNode1, TokenNode *tokenNode2, Machine *machine)
{
    if (tokenNode1 == NULL || tokenNode2 == NULL) // validação para garantir funcionamento
    {
        set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução SUB.", machine);
        return;
    }

    int number = 0;

    if (tokenNode1->token->type == HEXADECIMAL && (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // SUB HEX,REG
    {
        number = strtol(tokenNode1->token->lexeme, NULL, 16); // obtem o valor decimal através do hex fornecido
    }
    else if ((tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E) &&
             (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // SUB REG,REG
    {
        int register_from = tokenNode1->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de origem no vetor
        number = machine->registers[register_from];                 // obtem o valor no registrador de origem
    }
    else // sintaxe incorreta
    {
        set_machine_response("InvalidArgumentException: Parâmetros inválidos para a instrução ADD.", machine);
        return;
    }

    int register_to = tokenNode2->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de destino no vetor
    int value = machine->registers[0] - number;               // valor do registrador A - valor do registrador de origem
    machine->registers[register_to] = check_value(value);     // atribui o valor corrigido ao registrador de destino

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void and(TokenNode * tokenNode1, TokenNode *tokenNode2, Machine *machine)
{
    if (tokenNode1 == NULL || tokenNode2 == NULL) // validação para garantir funcionamento
    {
        set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução AND.", machine);
        return;
    }

    int number = 0;

    if (tokenNode1->token->type == HEXADECIMAL && (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // AND HEX,REG
    {
        number = strtol(tokenNode1->token->lexeme, NULL, 16); // obtem o valor decimal através do hex fornecido
    }
    else if ((tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E) &&
             (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // AND REG,REG
    {
        int register_from = tokenNode1->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de origem no vetor
        number = machine->registers[register_from];                 // obtem o valor no registrador de origem
    }
    else // sintaxe incorreta
    {
        set_machine_response("InvalidArgumentException: Parâmetros inválidos para a instrução AND.", machine);
        return;
    }

    int register_to = tokenNode2->token->lexeme[0] - A_ASCII; // obtem a posição do registrador no vetor
    int value = number & machine->registers[register_to];     // aplica a operação lógica E entre o número obtido e o registrador de destino
    machine->registers[register_to] = check_value(value);     // atribui o valor corrigido ao registrador de destino

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void or(TokenNode * tokenNode1, TokenNode *tokenNode2, Machine *machine)
{
    if (tokenNode1 == NULL || tokenNode2 == NULL) // validação para garantir funcionamento
    {
        set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução OR.", machine);
        return;
    }

    int number = 0;

    if (tokenNode1->token->type == HEXADECIMAL && (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // OR HEX,REG
    {
        number = strtol(tokenNode1->token->lexeme, NULL, 16); // obtem o valor decimal através do hex fornecido
    }
    else if ((tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E) &&
             (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // OR REG,REG
    {
        int register_from = tokenNode1->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de origem no vetor
        number = machine->registers[register_from];                 // obtem o valor no registrador de origem
    }
    else // sintaxe incorreta
    {
        set_machine_response("InvalidArgumentException: Parâmetros inválidos para a instrução OR.", machine);
        return;
    }

    int register_to = tokenNode2->token->lexeme[0] - A_ASCII; // obtem a posição do registrador no vetor
    int value = number | machine->registers[register_to];     // aplica a operação lógica OU entre o número obtido e o registrador de destino
    machine->registers[register_to] = check_value(value);     // atribui o valor corrigido ao registrador de destino

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void xor(TokenNode * tokenNode1, TokenNode *tokenNode2, Machine *machine) {
    if (tokenNode1 == NULL || tokenNode2 == NULL) // validação para garantir funcionamento
    {
        set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução XOR.", machine);
        return;
    }

    int number = 0;

    if (tokenNode1->token->type == HEXADECIMAL && (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // XOR HEX,REG
    {
        number = strtol(tokenNode1->token->lexeme, NULL, 16); // obtem o valor decimal através do hex fornecido
    }
    else if ((tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E) &&
             (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // XOR REG,REG
    {
        int register_from = tokenNode1->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de origem no vetor
        number = machine->registers[register_from];                 // obtem o valor no registrador de origem
    }
    else // sintaxe incorreta
    {
        set_machine_response("InvalidArgumentException: Parâmetros inválidos para a instrução XOR.", machine);
        return;
    }

    int register_to = tokenNode2->token->lexeme[0] - A_ASCII; // obtem a posição do registrador no vetor
    int value = number ^ machine->registers[register_to];     // aplica a operação lógica OU EXCLUSIVO entre o número obtido e o registrador de destino
    machine->registers[register_to] = check_value(value);     // atribui o valor corrigido ao registrador de destino

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void not(TokenNode * tokenNode1, TokenNode *tokenNode2, Machine *machine)
{
    if (tokenNode1 == NULL || tokenNode2 == NULL) // validação para garantir funcionamento
    {
        set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução NOT.", machine);
        return;
    }

    int number = 0;

    if (tokenNode1->token->type == HEXADECIMAL && (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // NOT HEX,REG
    {
        number = strtol(tokenNode1->token->lexeme, NULL, 16); // obtem o valor decimal através do hex fornecido
    }
    else if ((tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E) &&
             (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // NOT REG,REG
    {
        int register_from = tokenNode1->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de origem no vetor
        number = machine->registers[register_from];                 // obtem o valor no registrador de origem
    }
    else // sintaxe incorreta
    {
        set_machine_response("InvalidArgumentException: Parâmetros inválidos para a instrução XOR.", machine);
        return;
    }

    int register_to = tokenNode2->token->lexeme[0] - A_ASCII; // obtem a posição do registrador no vetor
    int value = ~number;                                      // aplica a operação lógica NÃO no valor obtido
    machine->registers[register_to] = check_value(value);     // atribui o valor corrigido ao registrador de destino

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void inc(TokenNode *tokenNode1, TokenNode *tokenNode2, Machine *machine)
{
    if (tokenNode1 == NULL || tokenNode2 == NULL) // validação para garantir funcionamento
    {
        set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução INC.", machine);
        return;
    }

    if (!(tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E) ||
        !(tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // sintaxe incorreta (esperado: INC REG,REG)
    {
        set_machine_response("InvalidArgumentException: Parâmetros inválidos para a instrução INC.", machine);
        return;
    }

    int register_from = tokenNode1->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de origem no vetor
    int register_to = tokenNode2->token->lexeme[0] - A_ASCII;   // obtem a posição do registrador de destino no vetor
    int value = machine->registers[register_from] + 1;          // incrementa uma unidade ao valor ao valor do registrador de origem
    machine->registers[register_to] = check_value(value);       // atribui o valor corrigido ao registrador de destino

    check_flag_c(value, machine);
    check_flag_z(value, machine);
}

void mov_output(TokenNode *tokenNode1, TokenNode *tokenNode2, Machine *machine)
{
    int output_addr;
    char *command = "i2c set -b";

    sscanf(tokenNode2->token->lexeme, "OUT%d", &output_addr);   // obtem o endereço da porta OUTPUT
    int register_from = tokenNode1->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de destino no vetor

    int length = snprintf(NULL, 0, "%s %d -a 0x%d 0x%X", command, CONFIG_EXAMPLES_M3P_I2C_BUS, (MIN_OUT_ADDR + output_addr), machine->registers[register_from]) + 1;
    char *buffer = (char *)malloc(length * sizeof(char));

    sprintf(buffer, "%s %d -a 0x%d 0x%X", command, CONFIG_EXAMPLES_M3P_I2C_BUS, (MIN_OUT_ADDR + output_addr), machine->registers[register_from]); // monta o comando para ler a porta i2c correspondente
    if (system(buffer) != 0)                                                                                                                      // envia o comando para o NuttX
    {
        set_machine_response("Não foi possível escrever na porta OUTPUT.", machine);
    }
}

void mov_input(TokenNode *tokenNode1, TokenNode *tokenNode2, Machine *machine)
{
    int input_addr, value;
    FILE *file;
    char *command = "i2c get -b 0 -a";
    char response[100];

    sscanf(tokenNode1->token->lexeme, "IN%d", &input_addr);   // obtem o endereço da porta INPUT
    int register_to = tokenNode2->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de destino no vetor

    int length = snprintf(NULL, 0, "%s 0x%d", command, (MIN_IN_ADDR + input_addr)) + 1;
    char *buffer = (char *)malloc(length * sizeof(char));

    sprintf(buffer, "%s 0x%d", command, (MIN_IN_ADDR + input_addr)); // monta o comando para ler a porta i2c correspondente

    file = popen(buffer, "r");
    if (file == NULL)
    {
        set_machine_response("Não foi possível ler a porta INPUT.", machine);
    }
    else
    {
        while (fgets(response, sizeof(response), file) != NULL) // resgata a resposta do console
        {
            if (sscanf(response, "READ %*s %*s %*s %*s %*s %*s Value: %d", &value) == 1) // obtem o valor da porta de entrada
            {
                machine->registers[register_to] = check_value((int)value); // atribui o valor corrigido ao registrador de destino

                check_flag_c((int)value, machine);
                check_flag_z((int)value, machine);
            }
            else
            {
                printf("Não foi possível ler a porta IN%d.\n", input_addr);
            }
        }
        pclose(file);
    }
}

void mov_memory_by_hex(TokenNode *tokenNode1, TokenNode *tokenNode2, Machine *machine)
{
    int register_from = 0, number = 0, memory_addr = 0;
    unsigned int hex;

    if ((tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E)) // MOV REG,#HEX
    {
        register_from = tokenNode1->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de origem no vetor
        number = machine->registers[register_from];             // obtem o valor do registrador de origem
    }
    else // MOV HEX,#HEX
    {
        number = strtol(tokenNode1->token->lexeme, NULL, 16); // obtem o valor decimal através do hex fornecido
    }

    sscanf(tokenNode2->token->lexeme, "#%X", &hex); // obtem o valor hexadecimal do endereço da memória enviado no formato #HEX
    memory_addr = (int)hex;                         // obtem o endereço através do hex fornecido

    add_number_at(number, memory_addr, &(machine->memory)); // adiciona o valor no endereço da memória
}

void mov_memory_by_register(TokenNode *tokenNode1, TokenNode *tokenNode2, Machine *machine)
{
    int register_from = 0, register_to = 0, number = 0, memory_addr = 0;

    if ((tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E)) // MOV REG,#REG
    {
        register_from = tokenNode1->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de origem no vetor
        number = machine->registers[register_from];             // obtem o valor do registrador de origem
    }
    else // MOV HEX,#REG
    {
        number = strtol(tokenNode1->token->lexeme, NULL, 16); // obtem o valor decimal através do hex fornecido
    }

    tokenNode2 = get_next_valid_token(tokenNode2, machine); // avança para o registrador de destino
    register_to = tokenNode2->token->lexeme[0] - A_ASCII;   // obtem a posição do registrador de destino no vetor
    memory_addr = machine->registers[register_to];          // obtem o valor do endereço da memória

    add_number_at(number, memory_addr, &(machine->memory)); // adiciona o valor no endereço da memória
}

void mov(TokenNode *tokenNode1, TokenNode *tokenNode2, Machine *machine)
{
    if (tokenNode1 == NULL || tokenNode2 == NULL) // validação para garantir funcionamento
    {
        set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução MOV.", machine);
        return;
    }

    int register_from = 0, number = 0, memory_addr = 0;
    unsigned int hex;

    if (tokenNode1->token->type == INPUT_PORT && (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // MOV INPUT,REG
    {
        mov_input(tokenNode1, tokenNode2, machine);
        return;
    }

    if ((tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E) && tokenNode2->token->type == OUTPUT_PORT) // MOV REG,OUTPUT
    {
        mov_output(tokenNode1, tokenNode2, machine);
        return;
    }

    if ((tokenNode1->token->type == HEXADECIMAL || (tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E)) && tokenNode2->token->type == RAM) // MOV HEX,#HEX || MOV REG,#HEX
    {
        mov_memory_by_hex(tokenNode1, tokenNode2, machine);
        return;
    }

    if ((tokenNode1->token->type == HEXADECIMAL || (tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E)) && tokenNode2->token->type == DRAM) // MOV HEX,#REG || MOV REG,#REG
    {
        mov_memory_by_register(tokenNode1, tokenNode2, machine);
        return;
    }

    if (tokenNode1->token->type == RAM && (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // MOV #HEX, REG
    {
        sscanf(tokenNode1->token->lexeme, "#%X", &hex);          // obtem o valor hexadecimal enviado no formato #HEX
        memory_addr = (int)hex;                                  // converte o valor para inteiro
        number = get_number_at(memory_addr, &(machine->memory)); // atribui o valor que estiver no endereço da memória fornecido
    }
    else if (tokenNode1->token->type == DRAM && (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // MOV #REG, REG
    {
        tokenNode1 = tokenNode2;                                // resgata o registrador de origem (#REG)
        tokenNode2 = get_next_valid_token(tokenNode2, machine); // avança para o registrador de destino

        register_from = tokenNode1->token->lexeme[0] - A_ASCII;  // obtem a posição do registrador de origem no vetor
        memory_addr = machine->registers[register_from];         // obtem o valor do registrador de origem
        number = get_number_at(memory_addr, &(machine->memory)); // obtem o valor que estiver no endereço da memória com valor igual ao do registrador de origem
    }
    else if (tokenNode1->token->type == HEXADECIMAL && (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // MOV HEX,REG
    {
        number = strtol(tokenNode1->token->lexeme, NULL, 16); // obtem o valor decimal através do hex fornecido
    }
    else if ((tokenNode1->token->type >= REGISTER_A && tokenNode1->token->type <= REGISTER_E) &&
             (tokenNode2->token->type >= REGISTER_A && tokenNode2->token->type <= REGISTER_E)) // MOV REG,REG
    {
        register_from = tokenNode1->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de origem no vetor
        number = machine->registers[register_from];             // obtem o valor no registrador de origem
    }
    else // sintaxe incorreta
    {
        set_machine_response("InvalidArgumentException: Parâmetros inválidos para a instrução MOV.", machine);
        return;
    }

    int register_to = tokenNode2->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de destino no vetor
    machine->registers[register_to] = check_value(number);    // atribui o valor corrigido ao registrador de destino

    check_flag_c(number, machine);
    check_flag_z(number, machine);
}

TokenNode *call(TokenNode *tokenNode, Machine *machine)
{
    if (tokenNode == NULL) // validação para garantir funcionamento
    {
        set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução CALL.", machine);
        return NULL;
    }

    int position = tokenNode->position;                     // obtem a posição do token no código
    if (stack_push(&(machine->call_stack), position) == -1) // adiciona a posição na pilha de chamadas
    {
        set_machine_response("StackOverflow: Pilha de chamadas sobrecarregada.", machine);
        return NULL;
    }

    return get_token_by_label(tokenNode->token, machine); // obtem o token correspondente ao label fornecido
}

TokenNode *ret(Machine *machine)
{
    int position = stack_pop(&(machine->call_stack)); // obtem a posição do token para qual o fluxo de execução será direcionado

    if (position == -1)
    {
        set_machine_response("StackUnderflow: Pilha de chamadas vazia.", machine);
        return NULL;
    }

    return get_token_at(position, machine); // obtem o token na respectiva posição
}

void push(TokenNode *tokenNode, Machine *machine)
{
    if (tokenNode == NULL) // validação para garantir funcionamento
    {
        set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução PUSH.", machine);
        return;
    }

    int number = strtol(tokenNode->token->lexeme, NULL, 16); // obtem o decimal a partir do hex fornecido

    if (stack_push(&(machine->data_stack), check_value(number)) == -1) // adiciona o valor na pilha de dados
    {
        set_machine_response("StackOverflow: Pilha de dados sobrecarregada.", machine);
    }

    check_flag_c(number, machine);
    check_flag_z(number, machine);
}

void pop(TokenNode *tokenNode, Machine *machine)
{
    if (tokenNode == NULL) // validação para garantir funcionamento
    {
        set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução POP.", machine);
        return;
    }

    int number = stack_pop(&(machine->data_stack)); // retira o valor do topo da pilha

    if (number == -1) // verificaa se a pilha está vazia
    {
        set_machine_response("StackUnderflow: Pilha de dados vazia.", machine);
        return;
    }

    int register_to = tokenNode->token->lexeme[0] - A_ASCII; // obtem a posição do registrador de destino no vetor
    machine->registers[register_to] = check_value(number);   // atribui o valor corrigido ao registrador de destino

    check_flag_c(number, machine);
    check_flag_z(number, machine);
}

HttpResponse *execute(Machine *machine)
{
    time_t start_time = time(NULL);
    TokenNode *tokenNode1 = NULL, *tokenNode2 = NULL, *current = machine->tokenNode_head;

    while (current != NULL)
    {
        switch (current->token->type)
        {
        case INSTRUCTION_ADD: // soma tokenNode1 com o registrador A e atribui em tokenNode2
            current = get_next_valid_token(current, machine);
            tokenNode1 = current;

            current = get_next_valid_token(current, machine);
            tokenNode2 = current;

            add(tokenNode1, tokenNode2, machine);
            break;

        case INSTRUCTION_SUB: // subtrai tokenNode1 com o registrador A e atribui em tokenNode2
            current = get_next_valid_token(current, machine);
            tokenNode1 = current;

            current = get_next_valid_token(current, machine);
            tokenNode2 = current;

            sub(tokenNode1, tokenNode2, machine);
            break;

        case INSTRUCTION_AND: // aplica a operação lógica E entre o tokenNode1 e o tokenNode2, atribuindo o resultado em tokenNode2
            current = get_next_valid_token(current, machine);
            tokenNode1 = current;

            current = get_next_valid_token(current, machine);
            tokenNode2 = current;

            and(tokenNode1, tokenNode2, machine);
            break;

        case INSTRUCTION_OR: // aplica a operação lógica OU entre o tokenNode1 e o tokenNode2, atribuindo o resultado em tokenNode2
            current = get_next_valid_token(current, machine);
            tokenNode1 = current;

            current = get_next_valid_token(current, machine);
            tokenNode2 = current;

            or(tokenNode1, tokenNode2, machine);

            break;

        case INSTRUCTION_XOR: // aplica a operação lógica OU EXCLUSIVO entre o tokenNode1 e o tokenNode2, atribuindo o resultado em tokenNode2
            current = get_next_valid_token(current, machine);
            tokenNode1 = current;

            current = get_next_valid_token(current, machine);
            tokenNode2 = current;

            xor(tokenNode1, tokenNode2, machine);
            break;

        case INSTRUCTION_NOT: // aplica a operação lógica NÃO em tokenNode1 e atribui o resultado em tokenNode2
            current = get_next_valid_token(current, machine);
            tokenNode1 = current;

            current = get_next_valid_token(current, machine);
            tokenNode2 = current;

            not(tokenNode1, tokenNode2, machine);
            break;

        case INSTRUCTION_MOV: // envia um valor de um endereço para outro
            current = get_next_valid_token(current, machine);
            tokenNode1 = current;

            current = get_next_valid_token(current, machine);
            tokenNode2 = current;

            mov(tokenNode1, tokenNode2, machine);
            break;

        case INSTRUCTION_INC: // incrementa uma unidade ao valor de tokenNode1 e atribui em tokenNode2
            current = get_next_valid_token(current, machine);
            tokenNode1 = current;

            current = get_next_valid_token(current, machine);
            tokenNode2 = current;

            inc(tokenNode1, tokenNode2, machine);
            break;

        case INSTRUCTION_JMP: // altera o fluxo de execução para o label parametrizado
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução JMP.", machine);
                break;
            }

            current = get_token_by_label(current->token, machine);
            break;

        case INSTRUCTION_JMPC: // altera o fluxo de execução para o label parametrizado caso a flag C esteja ativa (true)
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução JMPC.", machine);
                break;
            }

            if (machine->flag_c == true)
            {
                current = get_token_by_label(current->token, machine);
            }
            break;

        case INSTRUCTION_JMPZ: // altera o fluxo de execução para o label parametrizado caso a flag z esteja ativa (true)
            current = get_next_valid_token(current, machine);
            if (current == NULL)
            {
                set_machine_response("NullPointerException: Nenhum token encontrado para interpretar a instrução JMPZ.", machine);
                break;
            }

            if (machine->flag_z == true)
            {
                current = get_token_by_label(current->token, machine);
            }
            break;

        case INSTRUCTION_CALL: // altera o fluxo de execução para o label parametrizado
            current = get_next_valid_token(current, machine);

            current = call(current, machine);
            break;

        case INSTRUCTION_RET: // retorna o fluxo de execução para a próxima instrução após a respectiva chamada (CALL)
            current = ret(machine);
            break;

        case INSTRUCTION_PUSH: // adiciona na pilha o valor fornecido
            current = get_next_valid_token(current, machine);

            push(current, machine);
            break;

        case INSTRUCTION_POP: // remove da memória o valor mais ao topo e atribui ao registrador fornecido
            current = get_next_valid_token(current, machine);

            pop(current, machine);
            break;

        default: // entrará aqui em caso de virgula, dois pontos, label ou qualquer outro token não mapeado
            if (current == NULL)
            {
                set_machine_response("NullPointerException: Nenhum token encontrado para interpretar.", machine);
                break;
            }
            break;
        }

        // interrompe a execucao se identificar algum erro
        if (machine->message != NULL)
        {
            break;
        }

        // interrompe a execucao se current for NULL
        if (current == NULL)
        {
            set_machine_response("NullPointerException: Execução interrompida devido a erro inesperado.", machine);
            break;
        }

        // interrompe a execucao se o tempo limite exceder
        time_t current_time = time(NULL);
        if ((current_time - start_time) >= machine->timer)
        {
            set_machine_response("Tempo limite atingido. Programa encerrado.", machine);
            break;
        }

        delay(machine->delay);
        current = get_next_valid_token(current, machine); // retorna o próximo token
    }

    if (machine->message != NULL)
    {
        return httpResponse_create(machine->message, 400, "Bad Request");
    }

    return httpResponse_create("Execução concluída com suscesso.", 200, "OK");
}
