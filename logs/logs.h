#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>


void init_clock();
double get_elapsed_time();
void write_to_log(int n, int pid, long tid, int duration, int pl, char inst[]);