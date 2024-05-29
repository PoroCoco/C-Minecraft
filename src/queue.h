#pragma once

#include <stdint.h>
#include <stdbool.h>
#ifdef _WIN32
    #include <windows.h>
#elif __linux__
    #include <pthread.h>
#endif

typedef struct queue {
    void ** container;
    uint64_t size;
    uint64_t front;
    uint64_t count;
    uint64_t back;
    #ifdef _WIN32
        HANDLE mutex;
    #elif __linux__
        pthread_mutex_t mutex;
    #endif
} queue;


queue * queue_init(uint64_t intial_size);
void queue_enqueue(queue* q, void* value);
void *queue_dequeue(queue* q);
bool queue_is_empty(queue* q);
bool queue_is_full(queue* q);
void queue_cleanup(queue* q);