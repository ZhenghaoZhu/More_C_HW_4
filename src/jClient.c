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
#include <poll.h>

#include <sys/resource.h>

#include "jClient.h"
#include "ajs.h"

int jClientMain(){
    fprintf(stdout, "Inside client\n");

    fprintf(stdout, "before while loop1\n");
    int writeTo = open(C_TO_S, O_WRONLY);
    if(writeTo == -1){
        perror("Invalid FIFO");
        exit(EXIT_FAILURE);
    }
    int readFrom = open(S_TO_C, O_RDONLY);
    if(readFrom == -1){
        perror("Invalid FIFO");
        exit(EXIT_FAILURE);
    }
    struct pollfd * pd = malloc(sizeof(struct pollfd));
    //malloc for poll
    if(pd == NULL){
        perror("Invalid malloc");
        exit(EXIT_FAILURE);
    }
    pd->fd = writeTo;
    pd->events = POLLOUT;
    nfds_t t = 1;

    struct pollfd * turn = malloc(sizeof(struct pollfd));
    //malloc for poll
    if(turn == NULL){
        perror("Invalid malloc");
        exit(EXIT_FAILURE);
    }
    turn->fd = readFrom;
    turn->events = POLLIN;

    fprintf(stdout, "before while loop\n");
    while(1){
        char * args[20];
        int polla = poll(pd, t, -1);
        if(polla == -1){
            fprintf(stdout, "poll error\n");
        }
        fprintf(stdout, "Enter a cmd: ");
        ssize_t a;
        size_t size = 0;
        char * mal = NULL;
        //read a line from variable in
        if((a = getline(&mal, &size, stdin)) == -1){ //if no line is read (for non interactive mode mostly)
            free(mal);
            fprintf(stdout, "\n");
            exit(0);
        }
        fprintf(stderr,"Size of message: %ld\n", size);
        char * wholeCmd = malloc(a+1);
        strcpy(wholeCmd, mal);

        int index = 0;
        char * token = strtok(mal, " ");
        char * first = token;
        while(token != NULL){
            args[index] = token;
            fprintf(stderr, "Index %d: %s\n",index,args[index]);
            token = strtok(NULL, " ");
            index += 1;
        }
        args[index] = NULL;
        if(strncmp(first,"exit",4) == 0){
            if(write(writeTo, "1", 1) == -1){
                fprintf(stderr, "Server not active!\n");
            }
            close(writeTo);
            break;
        }
        else if(strncmp(first, "list",4) == 0){
            fprintf(stdout, "Listing all jobs!\n");
            if(write(writeTo, "2", 1) == -1){
                fprintf(stderr, "Server not active!\n");
            }

            int loopC = 0;
            char * ret = malloc(100);
            if(ret == NULL){
                continue;
            }
            if(read(readFrom, &loopC, sizeof(loopC)) == -1){
                fprintf(stderr, "Invalid read from server\n");
            }
            // int new = atoi(loopC);
            fprintf(stdout, "Loop C: %d\n", loopC);
            fprintf(stdout, "List:\n");
            for(int j = 0; j < loopC; j++){
                int bytesComing = 0;
                //read how many bytes is each line in the list
                if(read(readFrom, &bytesComing, sizeof(bytesComing)) == -1){
                    fprintf(stderr, "Invalid read from server\n");
                }
                if(read(readFrom, ret, bytesComing) == -1){
                    fprintf(stderr, "Invalid read from server\n");
                }
                fprintf(stdout, "%s\n", ret);

                int tempInt;
            }
        }
        else if(strncmp(first, "submit",6) == 0){
            if(write(writeTo, "3", 1) == -1){
                fprintf(stderr, "Server not active!\n");
            }
            //tells server how long the command is
            fprintf(stdout, "CMD: %s\n", wholeCmd);
            int cmdSize = a + 1;
            if(write(writeTo, &cmdSize , sizeof(cmdSize)) == -1){
                fprintf(stderr, "Server not active!\n");
            }
            if(write(writeTo, wholeCmd, cmdSize) == -1){
                fprintf(stderr, "Server not active!\n");
            }

            //write cmds to server //TODO
            int ser = poll(turn, t, -1);
            if(ser == -1){
                fprintf(stdout, "poll error\n");
            }
            char * ret = malloc(10);
            if(read(readFrom, ret, 1) == -1){
                fprintf(stderr, "Read no good!\n");
            }
            else {
                fprintf(stdout, "First byte: %s\n", ret);
                if(read(readFrom, ret, 1) == -1){
                    fprintf(stderr, "Read no good!\n");
                }
                else {
                    fprintf(stdout, "Second byte: %s\n", ret);
                }
            }
            
        }
        else if(strncmp(first, "get",3) == 0){
            if(write(writeTo, "4", 1) == -1){
                fprintf(stderr, "Server not active!\n");
            }
        }
        else if(strncmp(first, "kill",4) == 0){
            if(write(writeTo, "5", 1) == -1){
                fprintf(stderr, "Server not active!\n");
            }
        }
        else if(strncmp(first, "suspend",7) == 0){
            if(write(writeTo, "5", 1) == -1){
                fprintf(stderr, "Server not active!\n");
            }
        }
        else {
            // fprintf(stdout,"invalid cmd\n");
        }
        free(mal);
    }
    return 0;
}