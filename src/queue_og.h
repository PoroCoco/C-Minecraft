#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct queue_og {
    void ** container;
    uint64_t size;
    uint64_t front;
    uint64_t count;
    uint64_t back;
    pthread_mutex_t mutex;
} queue_og;


queue_og * queue_og_init(uint64_t intial_size);
void queue_og_enqueue_og(queue_og* q, void* value);
void *queue_og_dequeue_og(queue_og* q);
bool queue_og_is_empty(queue_og* q);
bool queue_og_is_full(queue_og* q);
void queue_og_cleanup(queue_og* q);