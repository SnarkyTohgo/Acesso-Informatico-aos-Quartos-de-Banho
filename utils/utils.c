#include "utils.h"
#include "macros.h"


int rng(int min, int max){
    return rand() % (max - min + 1) + min;
} 


int read_msg(int fd, char* msg){
    char ch;
    int idx = 0;

    while (read(fd, &ch, 1) > 0 && ch != '\n'){
        msg[idx] = ch;
        idx++;
    }

    msg[idx] = '\0';

    return strlen(msg);
}


int parse_arg_client(ClientArg* cargs, int argc, char* argv[]){
    
    if (argc != 4) 
        return ARGUMENT_ERROR;

    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-t") == EXIT_SUCCESS){
            if ((i + 1) == argc || cargs->nsecs > 0)
                return ARGUMENT_ERROR;
            
            if (atoi(argv[i + 1]) > 0){
                cargs->nsecs = atoi(argv[i + 1]);
                i++;
            }
            else 
                return SECONDS_ERROR;
        }
        else if(argv[i][0] != '-')
            strncpy(cargs->fifoname, argv[i], sizeof(cargs->fifoname));
        else 
            return ARGUMENT_ERROR;
    }

    if (cargs->fifoname[0] == '\0')
        return ARGUMENT_ERROR;

    return EXIT_SUCCESS;
}


int parse_arg_server(ServerArg* sargs, int argc, char* argv[]){
    if (argc > 8)
        return ARGUMENT_ERROR;
    
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-t") == EXIT_SUCCESS){
            if ((i + 1) == argc)
                return SECONDS_ERROR;

            if (atoi(argv[i + 1]) > 0){
                sargs->nsecs = atoi(argv[i + 1]);
                i++;
            }
            else 
                return SECONDS_ERROR;
        }
        else if (strcmp(argv[i], "-l") == EXIT_SUCCESS){
            if ((i + 1) == argc)
                return STALLS_ERROR;

            if (atoi(argv[i + 1]) > 0){
                sargs->nstalls = atoi(argv[i + 1]);
                i++;
            }
            else
                return STALLS_ERROR;
        }
        else if (strcmp(argv[i], "-n") == EXIT_SUCCESS){
            if ((i + 1) == argc)
                return THREADS_ERROR;

            if (atoi(argv[i + 1]) > 0){
                sargs->nthreads = atoi(argv[i + 1]);
                i++;
            }
            else 
                return THREADS_ERROR;
        }
        else if(argv[i][0] != '-' && sargs->fifoname[0] == '\0')
            strncpy(sargs->fifoname, argv[i], sizeof(sargs->fifoname));
        else 
            return ARGUMENT_ERROR;
    }

    if (sargs->fifoname[0] == '\0')
        return ARGUMENT_ERROR;

    return EXIT_SUCCESS;
}


void create_fifo(char* fifoname){

    if (mkfifo(fifoname, 0660) < 0){
        if (errno == EEXIST)
            fprintf(stderr, "FIFO <%s> already exists!\n", fifoname);
        else {
            fprintf(stderr, "FIFO error on creation");
            pthread_exit(NULL);
        }
    }
    else 
        fprintf(stderr, "Fifo <%s> successfully created!\n", fifoname);
}


void destroy_fifo(char* fifoname){
    if (unlink(fifoname) < 0){
        perror("FIFO Unlink");
        fprintf(stderr, "Error terminating FIFO %s\n", fifoname);
    }
    else 
        fprintf(stderr, "FIFO <%s> has been destroyed\n", fifoname);
}