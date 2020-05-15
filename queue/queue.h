#include "../utils/utils.h"

typedef struct Queue {
    unsigned front, back, size, capacity;
    unsigned* array;
} Queue;

Queue mkqueue(unsigned capacity);
 
bool is_full(Queue* queue);

bool is_empty(Queue* queue);

int occupy(Queue* q);

void unoccupy(Queue* queue, unsigned client);

void fill_queue(Queue* queue);
