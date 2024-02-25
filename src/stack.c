#include <stack.h>
#include <stdlib.h>
#include <stdio.h>

stack * stack_init(uint64_t size){
    stack *s = malloc(sizeof(*s));
    s->count = 0;
    s->size = size;
    s->container = malloc(sizeof(*s->container) * size);
    return s;
}

void stack_push(stack* s, void* value){
    if (s->count >= s->size){
        fprintf(stderr, "Tried to push onto a full stack !\n");
        return;
    }
    s->container[s->count++] = value;
}
void *stack_pop(stack* s){
    if (s->count == 0){
        fprintf(stderr, "Tried to pop an empty stack !\n");
        return NULL;
    }
    return s->container[--s->count];
}

bool stack_is_empty(stack const* s){
    return (s->count == 0);
}

bool stack_is_full(stack const* s){
    return (s->count == s->size);
}

void stack_cleanup(stack* s){
    free(s->container);
    free(s);
}