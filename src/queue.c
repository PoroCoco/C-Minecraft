#include <queue.h>
#include <stdlib.h>
#include <stdio.h>

queue * queue_init(uint64_t size){
    queue* q = malloc(sizeof(*q));
    q->container = malloc(sizeof(*q->container) * size);
    q->size = size;
    q->front = 0;
    q->back = 0;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->emptied, NULL);
    pthread_cond_init(&q->filled, NULL);
    return q;
}

bool _queue_is_full_lock_free(queue const* q){
    return ((q->back == q->front) && (q->count > 0));
}

bool _queue_is_empty_lock_free(queue const* q){
    return ((q->back == q->front) && (q->count == 0));
}

void queue_produce(queue* q, void* value){
    pthread_mutex_lock(&q->mutex);
    while (_queue_is_full_lock_free(q)){
        pthread_cond_wait(&q->emptied, &q->mutex);
    }
    q->container[q->back++] = value;
    q->back = q->back%q->size;
    q->count++;
    pthread_cond_signal(&q->filled);
    pthread_mutex_unlock(&q->mutex);
}

void *queue_consume(queue* q){
    pthread_mutex_lock(&q->mutex);
    while (_queue_is_empty_lock_free(q)){
        pthread_cond_wait(&q->filled, &q->mutex);
    }
    void * element = q->container[q->front++];
    q->front = q->front%q->size;
    q->count--;
    pthread_cond_signal(&q->emptied);
    pthread_mutex_unlock(&q->mutex);
    return element;
}

bool queue_is_full(queue * q){
    pthread_mutex_lock(&q->mutex);
    bool res = ((q->back == q->front) && (q->count > 0));
    pthread_mutex_unlock(&q->mutex);
    return res;
}

bool queue_is_empty(queue * q){
    pthread_mutex_lock(&q->mutex);
    bool res = ((q->back == q->front) && (q->count == 0));
    pthread_mutex_unlock(&q->mutex);
    return res;
}

void queue_cleanup(queue* q){
    free(q->container);
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->filled);
    pthread_cond_destroy(&q->emptied);
    free(q);
}