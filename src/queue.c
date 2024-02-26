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
    return q;
}

void queue_enqueue(queue* q, void* value){
    if (queue_is_full(q)){
        fprintf(stderr, "Tried to enqueue into a full queue !\n");
        return;
    }
    q->container[q->back++] = value;
    q->back = q->back%q->size;
    q->count++;
}

void *queue_dequeue(queue* q){
    if (queue_is_empty(q)){
        fprintf(stderr, "Tried to dequeue an empty queue !\n");
        return NULL;
    }
    void * element = q->container[q->front++];
    q->front = q->front%q->size;
    q->count--;
    return element;
}

bool queue_is_full(queue const* q){
    return ((q->back == q->front) && (q->count > 0));
}

bool queue_is_empty(queue const* q){
    return ((q->back == q->front) && (q->count == 0));
}

void queue_cleanup(queue* q){
    free(q->container);
    free(q);
}