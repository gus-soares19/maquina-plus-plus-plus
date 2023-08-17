#include <stdbool.h>
#include <stdio.h>
#define INITIAL_SIZE 100

typedef struct {
    int data[INITIAL_SIZE];
    int top;
} Stack;

void initializeStack(Stack* stack) {
    stack->top = -1;
}

bool isEmpty(Stack* stack) {
    return stack->top == -1;
}

void push(Stack* stack, int value) {
    stack->top++;
    stack->data[stack->top] = value;
}

void pushAt(Stack* stack, int index, int value) {
    if(index > stack->top){
        stack->top = index;
    }

    stack->data[index] = value;
}

int pop(Stack* stack) {
    if (isEmpty(stack)) {
        return -1;
    } else {
        int value = stack->data[stack->top];
        stack->top--;
        return value;
    }
}

int set(Stack* stack, int index, int value){
    int atPlace = stack->data[index];
    stack->data[index] = value;
    return atPlace;

}

int peek(Stack* stack) {
    if (isEmpty(stack)) {
        return -1;
    } else {
        return stack->data[stack->top];
    }
}

void clear(Stack* stack) {
    stack->top = -1;
}

void resize(Stack* stack, int newSize)
{
    if(newSize > (stack->top + 1))
    {
        Stack* newStack = (Stack*)malloc(sizeof(Stack));
        newStack->top = stack->top;

        for (int i = 0; i <= stack->top; i++) 
        {
            newStack->data[i] = stack->data[i];
        }

        stack = newStack;
    }

}
