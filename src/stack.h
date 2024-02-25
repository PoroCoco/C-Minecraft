#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct stack {
    void ** container;
    uint64_t count;
    uint64_t size;
} stack;


stack * stack_init(uint64_t size);
void stack_push(stack* s, void* value);
void *stack_pop(stack* s);
bool stack_is_empty(stack const* s);
bool stack_is_full(stack const* s);
void stack_cleanup(stack* s);