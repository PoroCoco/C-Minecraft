#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>


// Yes it's called a queue but if you try multiple concurrent enqueue on a full queue, then order is not guaranteed for them (surely I'll remember this)
typedef struct queue {
    void ** container;
    uint64_t size;
    uint64_t front;
    uint64_t count;
    uint64_t back;
    pthread_mutex_t mutex;
    pthread_cond_t emptied;
    pthread_cond_t filled;
} queue;


queue * queue_init(uint64_t intial_size);
void queue_produce(queue* q, void* value);
void *queue_consume(queue* q);
bool queue_is_empty(queue* q);
bool queue_is_full(queue* q);
void queue_cleanup(queue* q);