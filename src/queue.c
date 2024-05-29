#include <queue.h>
#include <stdlib.h>
#include <stdio.h>


void _queue_get_lock(queue* q){
    #ifdef _WIN32
        WaitForSingleObject(q->mutex, INFINITE);
    #elif __linux__
        pthread_mutex_lock(&q->mutex);
    #endif
}

void _queue_release_lock(queue* q){
    #ifdef _WIN32
        ReleaseMutex(q->mutex);
    #elif __linux__
        pthread_mutex_unlock(&q->mutex);
    #endif
}


queue * queue_init(uint64_t size){
    queue* q = malloc(sizeof(*q));
    q->container = malloc(sizeof(*q->container) * size);
    q->size = size;
    q->front = 0;
    q->back = 0;
    q->count = 0;
    #ifdef _WIN32
        q->mutex = CreateMutex(NULL, FALSE,NULL);
    #elif __linux__
        pthread_mutex_init(&q->mutex, NULL);
    #endif
    return q;
}

bool _queue_is_full_lock_free(queue const* q){
    return ((q->back == q->front) && (q->count > 0));
}

bool _queue_is_empty_lock_free(queue const* q){
    return ((q->back == q->front) && (q->count == 0));
}

void queue_enqueue(queue* q, void* value){
    _queue_get_lock(q);
    while(_queue_is_full_lock_free(q)){
        // fprintf(stderr, "Tried to enqueue into a full queue !\n");
        _queue_release_lock(q);
        _queue_get_lock(q);
    }
    q->container[q->back++] = value;
    q->back = q->back%q->size;
    q->count++;
    _queue_release_lock(q);
}

void *queue_dequeue(queue* q){
    _queue_get_lock(q);
    if (_queue_is_empty_lock_free(q)){
        fprintf(stderr, "Tried to dequeue an empty queue !\n");
        _queue_release_lock(q);
        return NULL;
    }
    void * element = q->container[q->front++];
    q->front = q->front%q->size;
    q->count--;
    _queue_release_lock(q);
    return element;
}

bool queue_is_full(queue * q){
    _queue_get_lock(q);
    bool res = ((q->back == q->front) && (q->count > 0));
    _queue_release_lock(q);
    return res;
}

bool queue_is_empty(queue * q){
    _queue_get_lock(q);
    bool res = ((q->back == q->front) && (q->count == 0));
    _queue_release_lock(q);
    return res;
}

void queue_cleanup(queue* q){
    free(q->container);
    // mutex destroy
    free(q);
}