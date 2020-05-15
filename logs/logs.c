#include "logs.h"

struct timeval start_time;
struct timeval curr_time;


void init_clock(){
    if (gettimeofday(&start_time, NULL) < 0)
        printf("Error getting start time\n");
}

double get_elapsed_time(){
    if (gettimeofday(&curr_time, NULL) < 0)
        printf("Error getting current time\n");

    double elapsed_time = (curr_time.tv_sec - start_time.tv_sec) * 1e6;
    elapsed_time += (curr_time.tv_usec - start_time.tv_usec);
    elapsed_time *= 1e-6;

    return elapsed_time;
}

void write_to_log(int n, int pid, long tid, int duration, int pl, char oper[]){
    fprintf(stdout, "%f ; %d ; %d ; %ld ; %d ; %d ; %s\n", 
        get_elapsed_time(), n, pid, tid, duration, pl, oper);
    
   fflush(stdout);
}