#include <stdbool.h>
#include <stdio.h>

#define STACK_SIZE 256

typedef struct
{
    int data[STACK_SIZE];
    int top;
} Stack;

void stack_init(Stack *stack)
{
    stack->top = -1;
}

void stack_free(Stack *stack)
{
    stack->top = -1;
}

bool is_empty(Stack *stack)
{
    return stack->top == -1;
}

bool is_full(Stack *stack)
{
    return stack->top >= STACK_SIZE;
}

void push(Stack *stack, int value)
{
    stack->top++;
    stack->data[stack->top] = value;
}

void push_at(Stack *stack, int index, int value)
{
    if (index > stack->top)
    {
        stack->top = index;
    }

    stack->data[index] = value;
}

int pop(Stack *stack)
{
    return is_empty(stack) ? -1 : stack->data[stack->top--];
}

void resize(Stack *stack, int newSize)
{
    if (newSize > (stack->top + 1))
    {
        Stack *new_stack = (Stack *)malloc(sizeof(Stack));
        new_stack->top = stack->top;

        for (int i = 0; i <= stack->top; i++)
        {
            new_stack->data[i] = stack->data[i];
        }

        stack = new_stack;
    }
}
