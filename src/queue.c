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
    mtx_init(&q->mutex, mtx_plain);
    return q;
}

bool _queue_is_full_lock_free(queue const* q){
    return ((q->back == q->front) && (q->count > 0));
}

bool _queue_is_empty_lock_free(queue const* q){
    return ((q->back == q->front) && (q->count == 0));
}

void queue_enqueue(queue* q, void* value){
    mtx_lock(&q->mutex);
    if (_queue_is_full_lock_free(q)){
        fprintf(stderr, "Tried to enqueue into a full queue !\n");
        mtx_unlock(&q->mutex);
        return;
    }
    q->container[q->back++] = value;
    q->back = q->back%q->size;
    q->count++;
    mtx_unlock(&q->mutex);
}

void *queue_dequeue(queue* q){
    mtx_lock(&q->mutex);
    if (_queue_is_empty_lock_free(q)){
        fprintf(stderr, "Tried to dequeue an empty queue !\n");
        mtx_unlock(&q->mutex);
        return NULL;
    }
    void * element = q->container[q->front++];
    q->front = q->front%q->size;
    q->count--;
    mtx_unlock(&q->mutex);
    return element;
}

bool queue_is_full(queue * q){
    mtx_lock(&q->mutex);
    bool res = ((q->back == q->front) && (q->count > 0));
    mtx_unlock(&q->mutex);
    return res;
}

bool queue_is_empty(queue * q){
    mtx_lock(&q->mutex);
    bool res = ((q->back == q->front) && (q->count == 0));
    mtx_unlock(&q->mutex);
    return res;
}

void queue_cleanup(queue* q){
    free(q->container);
    mtx_destroy(&q->mutex);
    free(q);
}