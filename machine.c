#include <string.h>

typedef struct
{
    int A;
    int B;
    int C;
    int D;
    int E;
} Machine;

add(int num, char* reg, Machine* machine)
{
    if(strcmp(reg, "A") == 0) // ADD NUM, A
    {
        machine->A += num;
    }
    else if(strcmp(reg, "B") == 0) // ADD NUM, B
    {
        machine->B += num;
    }
    else if(strcmp(reg, "C") == 0) // ADD NUM, C
    {
        machine->C += num;
    }
    else if(strcmp(reg, "D") == 0) // ADD NUM, D
    {
        machine->D += num;
    }
    else if(strcmp(reg, "E") == 0) // ADD NUM E
    {
        machine->E += num;
    }
}

sub(int num, char* reg, Machine* machine)
{
    if(strcmp(reg, "A") == 0) // SUB NUM, A
    {
        machine->A -= num;
    }
    else if(strcmp(reg, "B") == 0) // SUB NUM, B
    {
        machine->B -= num;
    }
    else if(strcmp(reg, "C") == 0) // SUB NUM, C
    {
        machine->C -= num;
    }
    else if(strcmp(reg, "D") == 0) // SUB NUM, D
    {
        machine->D -= num;
    }
    else if(strcmp(reg, "E") == 0) // SUB NUM, E
    {
        machine->E -= num;
    }
}

inc(char* reg, Machine* machine)
{
    if(strcmp(reg, "A") == 0) // INC A
    {
        machine->A++;
    }
    else if(strcmp(reg, "B") == 0) // INC B
    {
        machine->B++;
    }
    else if(strcmp(reg, "C") == 0) // INC C
    {
        machine->C++;
    }
    else if(strcmp(reg, "D") == 0) // INC D
    {
        machine->D++;
    }
    else if(strcmp(reg, "E") == 0) // INC E
    {
        machine->E++;
    }
}

mov(char* from, char* to, Machine* machine)
{
    if(strcmp(from, "A") == 0) // MOV A
    {
        if(strcmp(to, "B") == 0) // MOV A, B
        {
            machine->B = machine->A;
        }
        else if(strcmp(to, "C") == 0) // MOV A, C
        {
            machine->C = machine->A;
        }
        else if(strcmp(to, "D") == 0) // MOV A, D
        {
            machine->D = machine->A;
        }
        else if(strcmp(to, "E") == 0) // MOV A, E
        {
            machine->E = machine->A;
        }
    }

    else if(strcmp(from, "B") == 0) // MOV B
    {
        if(strcmp(to, "A") == 0) // MOV B, A
        {
            machine->A = machine->B;
        }
        else if(strcmp(to, "C") == 0) // MOV B, C
        {
            machine->C = machine->B;
        }
        else if(strcmp(to, "D") == 0) // MOV B, D
        {
            machine->D = machine->B;
        }
        else if(strcmp(to, "E") == 0) // MOV B, E
        {
            machine->E = machine->B;
        }
    }
        
    else if(strcmp(from, "C") == 0) // MOV C
    {
        if(strcmp(to, "A") == 0) // MOV C, A
        {
            machine->A = machine->C;
        }
        else if(strcmp(to, "B") == 0) // MOV C, B
        {
            machine->B = machine->C;
        }
        else if(strcmp(to, "D") == 0) // MOV C, D
        {
            machine->D = machine->C;
        }
        else if(strcmp(to, "E") == 0) // MOV C, E
        {
            machine->E = machine->C;
        }
    }

    else if(strcmp(from, "D") == 0) // MOV D
    {
        if(strcmp(to, "A") == 0) // MOV D, A
        {
            machine->A = machine->D;
        }
        else if(strcmp(to, "B") == 0) // MOV D, B
        {
            machine->B = machine->D;
        }
        else if(strcmp(to, "C") == 0) // MOV D, C
        {
            machine->C = machine->D;
        }
        else if(strcmp(to, "E") == 0) // MOV D, E
        {
            machine->E = machine->D;
        }
    }

    else if(strcmp(from, "E") == 0) // MOV E
    {
        if(strcmp(to, "A") == 0) // MOV E, A
        {
            machine->A = machine->E;
        }
        else if(strcmp(to, "B") == 0) // MOV E, B
        {
            machine->B = machine->E;
        }
        else if(strcmp(to, "C") == 0) // MOV E, C
        {
            machine->C = machine->E;
        }
        else if(strcmp(to, "D") == 0) // MOV E, D
        {
            machine->D = machine->E;
        }
    }
}

