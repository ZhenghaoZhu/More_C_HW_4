#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/resource.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <signal.h>

#include "jServer.h"
#include "ajs.h"

void sigChld_Handler(){
    pid_t id;
    int stat;
    if((id = waitpid(-1, &stat, WNOHANG)) != -1){
        fprintf(stdout, "child returned: %d\n", id);
    }
}

void initializeGlobals(){
    jobInfoLL[0].next = NULL;
}

int jServerMain(int max){
    initializeGlobals();
    // if(signal(SIGCHLD, sigChld_Handler) == SIG_ERR){
    //     fprintf(stdout, "Handler not set up correctly\n");
    // }
    fprintf(stdout, "Inside server\n");
    //variables
    int curNumJobs = 0;
    clock_t start = clock(), end; //cpu cycles
    double cpuTime;
    time_t timeStart = time(NULL); //time
    time_t timeEnd;
    //open special for reading
    fprintf(stdout, "before while loop1\n");
    int readFrom = open(C_TO_S, O_RDONLY);
    if(readFrom == -1){
        perror("Invalid FIFO");
        exit(EXIT_FAILURE);
    }
    int writeTo = open("tmp/.sTOc", O_WRONLY);
    if(writeTo == -1){
        perror("Invalid FIFO");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "before while loop2\n");
    //malloc for poll
    struct pollfd * pd = malloc(sizeof(struct pollfd));
    if(pd == NULL){
        perror("Invalid malloc");
        exit(EXIT_FAILURE);
    }
    pd->fd = readFrom;
    pd->events = POLLIN;
    nfds_t t = 1;
    

    int id = 0;
    fprintf(stdout, "before while loop\n");
    while(1){
        int polla = poll(pd, t, -1);
        // fprintf(stdout, "poll: %d\n", polla);
        if(polla == -1){
            fprintf(stdout, "poll error\n");
        }
        // fprintf(stdout, "revents : %d\n", pd->revents);
        if(pd->revents != 1){
            close(readFrom);
            readFrom = open("special", O_RDONLY);
            pd->fd = readFrom;
            pd->events = POLLIN;
            
            continue;
        }

        char * mal = NULL;
        // size_t size = 0;
        // ssize_t a;
        mal = malloc(50);
        if(read(readFrom, mal , 1) == -1){
            fprintf(stderr, "Reading error :( \n");
            free(mal);
            continue;
        }
        if(curNumJobs == max){
            fprintf(stdout, "Max jobs on server! %d\n", curNumJobs);
            continue;
        }
        fprintf(stdout, "read from client: %s\n", mal);
        if(strncmp(mal, "1",1) == 0){
            fprintf(stdout, "exiting\n");
            // free(mal);
            // continue;
        }
        else if(strncmp(mal, "2",1) == 0){//list all jobs
            fprintf(stdout, "Listing all jobs!\n");
            struct jobInfo * ptr = &jobInfoLL[0];
            ptr = ptr->next;
            char * f = (char *) malloc(100);
            while(ptr != &jobInfoLL[0]){
                
                int size = 10; //holder
                // int size = sprintf(f,);
                if(write(writeTo, f, size) == -1){
                    perror("Invalid write to client");
                }
                free(f);
                ptr = ptr->next;
            }
            // continue;
        }
        else if(strncmp(mal, "3",1) == 0){
            struct jobInfo * rock = malloc(sizeof(struct jobInfo));
            rock->cpuStart = clock();
            char fileID[50];
            if(sprintf(fileID, "%d.txt", id) == -1){
                fprintf(stderr, "sprintf failure\n");
            }
            int newF = open(fileID, O_WRONLY);
            if(newF == -1){
                perror("Can't open");
                free(mal);
                continue;
            }
            rock->timeStart = time(NULL);

            pid_t pid = fork();
            if(pid == 0){
                close(1); //close stdout
                if(dup(newF) == -1){
                    perror("dup");
                }
                if(system(mal) == -1){
                    fprintf(stderr, "System failed\n");
                }
                // if(execvp(token, cmdArgs) == -1){
                //     perror("Invalid command :3");
                //     exit(1);
                //     // debugExit(token, errno);
                // }

                fprintf(stdout, "Inside child\n");

                exit(0);
            }
            else{
                int stat;
                wait(&stat);
                end = clock();
                cpuTime = ((double) (end - start)) / CLOCKS_PER_SEC;
                timeEnd = time(NULL);
                fprintf(stdout, "Time: %ld\n", (timeEnd - timeStart));
                fprintf(stdout, "Inside parent, process CPU time = %f\n", cpuTime);
                char * ret = malloc(3);
                strcpy(ret, "10");
                if(write(writeTo, ret, 2) == -1){
                    fprintf(stderr, "Read no good!\n");
                }
            }
            fprintf(stdout, "output received! : %s\n", mal);
            curNumJobs += 1;
        }
        else if(strncmp(mal, "4",1) == 0){
            
        }
        else if(strncmp(mal, "5", 1) == 0){

        }
        else {
            fprintf(stdout, "Invalid 1st byte!\n");
        }
        free(mal);
    }
    return 0;
}