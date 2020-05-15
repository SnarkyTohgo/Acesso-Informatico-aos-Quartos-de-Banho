#include "queue.h"

Queue mkqueue(unsigned capacity){
    Queue queue;

    queue.capacity = capacity;
    queue.size = 0;
    queue.front = 0;
    queue.back = capacity - 1;
    queue.array = (unsigned*) malloc(queue.capacity * sizeof(unsigned));

    return queue;
}
 
bool is_full(Queue* queue){
    return queue->size == queue->capacity;
}

bool is_empty(Queue* queue){
    return queue->size == 0;
}

int occupy(Queue* queue){
    if (is_empty(queue))
        return QUEUE_IS_EMPTY;

    unsigned first_in = queue->array[queue->front];
    
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    return first_in;
}

void unoccupy(Queue* queue, unsigned client){
    if (is_full(queue))
        return;

    queue->back = (queue->back + 1) % queue->capacity;
    queue->array[queue->back] = client;
    queue->size++;
}

void fill_queue(Queue* queue){
    for (unsigned client = 1; client < queue->capacity; client++)
        unoccupy(queue, client);
}