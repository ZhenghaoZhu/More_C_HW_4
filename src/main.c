#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include "jClient.h"
#include "jServer.h"
#include "ajs.h"

int main(int argc, char *argv[]){
    if(argc <= 1){
        fprintf(stderr, "Please provide flags to specify server (-s) or client (-c). Also specify max number of jobs for server.\n");   
        exit(EXIT_FAILURE);
    }

    if(access(C_TO_S, F_OK) != 0){
        mkfifo(C_TO_S, 0666);
    }

    if(access(S_TO_C, F_OK) != 0){
        mkfifo(S_TO_C, 0666);
    }

    if(strncmp(argv[1], "-c", 2) == 0){
        if(argc == 3){
            jClientMain(argv[2]);
        } else {
            jClientMain(NULL);
        }
    }
    else if(strncmp(argv[1], "-s", 2) == 0){
        int maxNumberOfJobs = atoi(argv[2]);

        if(maxNumberOfJobs > 0){
            jServerMain(maxNumberOfJobs);
        }
        else {
            fprintf(stderr, "Invalid max number of jobs. Please provide a number greater than 0.\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        fprintf(stderr, "Please provide -s or -c flags to specify server or client respectively.\n");
    }
    return 0;   
}