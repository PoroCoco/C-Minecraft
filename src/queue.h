#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <threads.h>

typedef struct queue {
    void ** container;
    uint64_t size;
    uint64_t front;
    uint64_t count;
    uint64_t back;
    mtx_t mutex;
} queue;


queue * queue_init(uint64_t intial_size);
void queue_enqueue(queue* q, void* value);
void *queue_dequeue(queue* q);
bool queue_is_empty(queue const* q);
bool queue_is_full(queue const* q);
void queue_cleanup(queue* q);