#include "../utils/utils.h"
#include "../logs/logs.h"

int create_msg(char* msg, int n);

int mkrequest(const char* fifo, int duration, const void* msg);

void* thread_handler(void* arg);

void handle_fifo(char* fifo, int duration, int n);