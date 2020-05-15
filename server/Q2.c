#include "Q2.h"

// Globals

Queue queue;

int stall = 1, bathroom_time;
bool closed = false;
bool active_threads_reached_max = false;
bool active_stalls_reached_max = false;

sem_t max_threads_sem;
sem_t max_stalls_sem;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void* thread_handler(void* arg){  
    pthread_detach(pthread_self());

    int fdPrivFifo, n, duration, pid, this_stall, num_tries = 0;
    long tid;

    char priv_fifo[BUFFER_SIZE], client_msg[BUFFER_SIZE];

    sscanf((char*) arg, "[%d, %d, %ld, %d, -1]", &n, &pid, &tid, &duration);
    write_to_log(n, pid, tid, duration, -1, RECEIVED);

    snprintf(priv_fifo, sizeof(priv_fifo), "/tmp/%d.%ld", pid, tid);


    if (active_stalls_reached_max){
        sem_wait(&max_stalls_sem);

        pthread_mutex_lock(&mutex);
            this_stall = occupy(&queue);
        pthread_mutex_unlock(&mutex);
    } else {
        pthread_mutex_lock(&mutex);
            this_stall = stall;
            stall++;
        pthread_mutex_unlock(&mutex);
    }


    while ((fdPrivFifo = open(priv_fifo, O_WRONLY | O_NONBLOCK)) < 0 
            && num_tries < MAX_TRIES){
    
        fprintf(stderr, "Error opening private FIFO <%s>\n", priv_fifo);
        usleep(200);
        num_tries++;
    }


    if (num_tries == MAX_TRIES){
        write_to_log(n, pid, tid, duration, -1, GAVE_UP);
        
        release_threads_sem();
        release_stalls_sem(this_stall);

        pthread_exit(NULL);
    }


    if (get_elapsed_time() < bathroom_time){
        sprintf(client_msg, "[%d, %d, %ld, %d, %d]", n, getpid(), pthread_self(), duration, this_stall);
        write_to_log(n, getpid(), pthread_self(), duration, this_stall, ENTER);
    } else {
        sprintf(client_msg, "[%d, %d, %ld, %d, %d]", n, getpid(), pthread_self(), -1, -1);
        write_to_log(n, getpid(), pthread_self(), duration, this_stall, TOO_LATE);
    } 


    if (write(fdPrivFifo, &client_msg, BUFFER_SIZE) < 0){
        fprintf(stderr, "Error writig to private FIFO @%d\n", n);
        write_to_log(n, pid, tid, duration, -1, GAVE_UP);
        
        if (close(fdPrivFifo) < 0)
            fprintf(stderr, "Error closing private FIFO @%d\n", n);

        release_threads_sem();
        release_stalls_sem(this_stall);

        pthread_exit(NULL);
    }

    if (close(fdPrivFifo) < 0) {
        fprintf(stderr, "Error closing private FIFO @%d\n", n);
        pthread_exit(NULL);
    }

    
    if (!closed){
        usleep(duration * 1000);
        write_to_log(n, getpid(), pthread_self(), duration, this_stall, TIME_UP);
    } 
        
    release_threads_sem();
    release_stalls_sem(this_stall);

    pthread_exit(NULL);
}


void release_threads_sem(){
    if (active_threads_reached_max){
        if (sem_post(&max_threads_sem) < 0)
            perror("max_threads_sem");
    }
}


void release_stalls_sem(int stall){
    if (active_stalls_reached_max){
        pthread_mutex_lock(&mutex);
            unoccupy(&queue, stall);
        pthread_mutex_unlock(&mutex);

        if (sem_post(&max_stalls_sem) < 0)
            perror("max_stalls_sem");
    }
}


int main(int argc, char* argv[]){
    int fdPubFifo, ret_code;
    char msg[BUFFER_SIZE], public_fifo[BUFFER_SIZE] = "server/";

    // ----------------------
    // Parse Arguments

    ServerArg args;
    strcpy(args.fifoname, "");
    args.nsecs = 0; args.nstalls = 0; args.nthreads = 0;

    if ((ret_code = parse_arg_server(&args, argc, argv)) < 0){
        switch (ret_code){
            case ARGUMENT_ERROR:
                fprintf(stderr, "Argument error: Input");
                break;
            case SECONDS_ERROR:
                fprintf(stderr, "Argument error: Duration");
                break;
            case STALLS_ERROR:
                fprintf(stderr, "Argument error: Stall");
                break;
            case THREADS_ERROR:
                fprintf(stderr, "Argument error: Thread");
                break;
        }

        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "FIFO: %s\n", public_fifo);
    

    // ----------------------
    // Handle Public FIFO

    bathroom_time = args.nsecs;
    
    if (args.nstalls > 0)
        active_threads_reached_max = true;

    if (args.nthreads > 0)
        active_stalls_reached_max = true;

    init_clock();
    fprintf(stderr, "Execution time: %d\nFifo: %s\n", args.nsecs, args.fifoname);

    strcat(public_fifo, args.fifoname);
    create_fifo(public_fifo);


    if ((fdPubFifo = open(public_fifo, O_RDONLY, O_NONBLOCK)) != -1){
        fprintf(stderr, "Fifo <%s> opened successfully in READ ONLY mode.\n", public_fifo); 
    } else {
        fprintf(stderr, "Error opening FIFO <%s>.\n", public_fifo);
        destroy_fifo(public_fifo);
    }

    // ----------------------
    // Initialize Semaphores

    if (active_threads_reached_max)
        sem_init(&max_threads_sem, 0, args.nthreads);

    if (active_stalls_reached_max){
        sem_init(&max_stalls_sem, 0, args.nstalls);
        queue = mkqueue(args.nstalls);
        fill_queue(&queue);
    }

    // ----------------------
    // Generate Threads

    while (get_elapsed_time() < args.nsecs){
        if (read_msg(fdPubFifo, msg) > 0){
            if (msg[0] == '['){
                if (active_threads_reached_max)
                    sem_wait(&max_threads_sem);

                char* dup;
                dup = strdup(msg);
                
                pthread_t thread;
                pthread_create(&thread, NULL, thread_handler, dup);
            }
        }    
    }

    // ----------------------
    // Close Bathroom and Handle Last Requests

    closed = true;
    
    destroy_fifo(public_fifo);

    while (read_msg(fdPubFifo, msg) > 0){
        if (msg[0] == '['){
            if (active_threads_reached_max)
                sem_wait(&max_threads_sem);

            char* dup;
            dup = strdup(msg);
            
            pthread_t thread;
            pthread_create(&thread, NULL, thread_handler, dup);
        }
    }

    if (close(fdPubFifo) < 0)
         fprintf(stderr, "Error closing public FIFO\n"); 

    fprintf(stderr, "Bathroom is closed - %f\n", get_elapsed_time());

    pthread_exit(EXIT_SUCCESS);
}
