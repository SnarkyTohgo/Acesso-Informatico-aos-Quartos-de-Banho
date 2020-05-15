#include "U2.h"

// Globals
int count = 1;
bool closed = false;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void* thread_handler(void* arg){
    pthread_detach(pthread_self());

    char priv_fifo[BUFFER_SIZE], msg[BUFFER_SIZE], server_msg[BUFFER_SIZE];
    char* public_fifo = arg;

    int fdPubFifo, fdPrivFifo, duration, num_tries = 0;
    int i, pid, stall;
    long tid;
    

    // ----------------------
    // Make Request

    pthread_mutex_lock(&mutex);
        int n = count++;
    pthread_mutex_unlock(&mutex);   

    if ((fdPubFifo = open(public_fifo, O_WRONLY | O_NONBLOCK, 0660)) < 0){
        closed = true;
        
        write_to_log(n, getpid(), pthread_self(), -1, -1, CLOSED);
        fprintf(stderr, "Service is closed\n");

        pthread_exit(NULL);
    }

    snprintf(priv_fifo, sizeof(priv_fifo), "/tmp/%d.%ld", getpid(), pthread_self());
    create_fifo(priv_fifo);

    duration = create_msg(msg, n);

    if (write(fdPubFifo, &msg, ARRAY_SIZE(msg)) < 0){
        fprintf(stderr, "Error writing to public FIFO\n");
        write_to_log(n, getpid(), pthread_self(), -1, -1, FAILED);
        
        if (close(fdPubFifo) < 0)
            fprintf(stderr, "Error closing public FIFO\n");

        destroy_fifo(priv_fifo);       

        closed = true;
        pthread_exit(NULL);
    }
        
    write_to_log(n, getpid(), pthread_self(), duration, -1, I_WANT);
    if (close(fdPubFifo) < 0){
        fprintf(stderr, "Error closing public FIFO\n");
        pthread_exit(NULL);
    }

    // ----------------------
    // Handle Private FIFO

    if ((fdPrivFifo = open(priv_fifo, O_RDONLY)) < 0){
        fprintf(stderr, "Error opening FIFO <%s>\n", priv_fifo);
        destroy_fifo(priv_fifo); 
    }

    while (read(fdPrivFifo, server_msg, BUFFER_SIZE) <= 0 
            && num_tries < MAX_TRIES){

        fprintf(stderr, "Error reading message from server... retry\n");
        usleep(200);
        num_tries++;
    }    

    if (num_tries == MAX_TRIES){
        fprintf(stderr, "Unable to read from private FIFO\n");
        write_to_log(n, getpid(), pthread_self(), -1, -1, FAILED);

        if (close(fdPrivFifo) < 0)
            fprintf(stderr, "Error closing private FIFO\n");

        destroy_fifo(priv_fifo);
        pthread_exit(NULL);
    }

    sscanf(server_msg, "[%d, %d, %ld, %d, %d]", &i, &pid, &tid, &duration, &stall);

    if (stall < 0 && duration < 0) {
        closed = true;
        write_to_log(i, pid, tid, -1, -1, CLOSED);
    } else
        write_to_log(i, pid, tid, duration, stall, I_AM_IN);

    if (close(fdPrivFifo) < 0)
            fprintf(stderr, "Error closing private FIFO\n");

    destroy_fifo(priv_fifo);
    pthread_exit(NULL);
}

int create_msg(char* msg, int n){
    int duration = rng(1, 20);
    sprintf(msg, "[%d, %d, %ld, %d, -1]", n, (int)getpid(), (long)pthread_self(), duration);

    return duration;
}


int main(int argc, char* argv[]){
    
    ClientArg args;

    strcpy(args.fifoname, "");

    int t = 0, ret_code;

    srand(time(NULL));

    if ((ret_code = parse_arg_client(&args, argc, argv)) < 0){
        switch (ret_code){
            case ARGUMENT_ERROR:
                fprintf(stderr, "Input error\n");
                break;
            case SECONDS_ERROR:
                fprintf(stderr, "Duration error\n");
                break;
        }

        exit(EXIT_FAILURE);
    }

    init_clock();
    printf("Execution time: %d\nFifo: <%s>\n", args.nsecs, args.fifoname);

    char public_fifo[BUFFER_SIZE] = "server/";
    strcat(public_fifo, args.fifoname);

    while (get_elapsed_time() < args.nsecs && !closed){
        pthread_t thread;
        pthread_create(&thread, NULL, thread_handler, &public_fifo);
        t = (t + 1) % MAX_THREADS;

        usleep(10000);
    }

    printf("\nDone - %f seconds\n", get_elapsed_time());
    pthread_exit(EXIT_SUCCESS);
}
