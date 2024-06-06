#include <queue_og.h>
#include <stdlib.h>
#include <stdio.h>

queue_og * queue_og_init(uint64_t size){
    queue_og* q = malloc(sizeof(*q));
    q->container = malloc(sizeof(*q->container) * size);
    q->size = size;
    q->front = 0;
    q->back = 0;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    return q;
}

bool _queue_og_is_full_lock_free(queue_og const* q){
    return ((q->back == q->front) && (q->count > 0));
}

bool _queue_og_is_empty_lock_free(queue_og const* q){
    return ((q->back == q->front) && (q->count == 0));
}

void queue_og_enqueue_og(queue_og* q, void* value){
    pthread_mutex_lock(&q->mutex);
    while(_queue_og_is_full_lock_free(q)){
        // fprintf(stderr, "Tried to enqueue_og into a full queue_og !\n");
        pthread_mutex_unlock(&q->mutex);
        pthread_mutex_lock(&q->mutex);
    }
    q->container[q->back++] = value;
    q->back = q->back%q->size;
    q->count++;
    pthread_mutex_unlock(&q->mutex);
}

void *queue_og_dequeue_og(queue_og* q){
    pthread_mutex_lock(&q->mutex);
    if (_queue_og_is_empty_lock_free(q)){
        fprintf(stderr, "Tried to dequeue_og an empty queue_og !\n");
        pthread_mutex_unlock(&q->mutex);
        return NULL;
    }
    void * element = q->container[q->front++];
    q->front = q->front%q->size;
    q->count--;
    pthread_mutex_unlock(&q->mutex);
    return element;
}

bool queue_og_is_full(queue_og * q){
    pthread_mutex_lock(&q->mutex);
    bool res = ((q->back == q->front) && (q->count > 0));
    pthread_mutex_unlock(&q->mutex);
    return res;
}

bool queue_og_is_empty(queue_og * q){
    pthread_mutex_lock(&q->mutex);
    bool res = ((q->back == q->front) && (q->count == 0));
    pthread_mutex_unlock(&q->mutex);
    return res;
}

void queue_og_cleanup(queue_og* q){
    free(q->container);
    pthread_mutex_destroy(&q->mutex);
    free(q);
}