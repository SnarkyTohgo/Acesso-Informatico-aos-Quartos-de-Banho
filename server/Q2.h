#include "../utils/utils.h"
#include "../logs/logs.h"
#include "../queue/queue.h"

void* thread_handler(void* arg);

void release_threads_sem();
void release_stalls_sem(int stall);


