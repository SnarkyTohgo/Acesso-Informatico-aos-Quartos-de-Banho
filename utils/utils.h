#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <limits.h>

#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#include "macros.h"


#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))


typedef struct ClientArg {
    unsigned int nsecs;
    char fifoname[BUFFER_SIZE];
} ClientArg;

typedef struct ServerArg {   
    unsigned int nsecs;
    unsigned int nstalls;
    unsigned int nthreads;
    char fifoname[BUFFER_SIZE];
} ServerArg;


int rng(int min, int max); 
int read_msg(int fd, char* msg);

int parse_arg_client(ClientArg* cargs, int argc, char* argv[]);
int parse_arg_server(ServerArg* sargs, int argc, char* argv[]);

void create_fifo(char* fifoname);
void destroy_fifo(char* fifoname);

