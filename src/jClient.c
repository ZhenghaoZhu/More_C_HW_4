#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
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
#include <poll.h>

#include "ajs.h"
#include "jClient.h"

int jClientMain(char* nonInteractiveFile){
    int writeToServer = open(C_TO_S, O_WRONLY);
    if(writeToServer == -1){
        perror("Invalid FIFO");
        exit(EXIT_FAILURE);
    }
    int readFromServer = open(S_TO_C, O_RDONLY);
    if(readFromServer == -1){
        perror("Invalid FIFO");
        exit(EXIT_FAILURE);
    }
    struct pollfd * writeToPD = malloc(sizeof(struct pollfd));
    //malloc for poll
    if(writeToPD == NULL){
        perror("Invalid malloc");
        exit(EXIT_FAILURE);
    }
    writeToPD->fd = writeToServer;
    writeToPD->events = POLLOUT;
    nfds_t t = 1;

    struct pollfd * readFromPD = malloc(sizeof(struct pollfd));
    //malloc for poll
    if(readFromPD == NULL){
        perror("Invalid malloc");
        exit(EXIT_FAILURE);
    }
    readFromPD->fd = readFromServer;
    readFromPD->events = POLLIN;

    fprintf(stdout, "Client Connected to Server!\n");
    while(true){
        int polla = poll(writeToPD, t, -1);
        if(polla == -1){
            fprintf(stdout, "poll error\n");
        }
        fprintf(stdout, "client> ");
        ssize_t getLineRet;
        size_t size = 0;
        char * cmdArgsMalloc = NULL;
        int newStdIn = 1;
        if(nonInteractiveFile != NULL && access(nonInteractiveFile, F_OK) == 0){
            newStdIn = open(nonInteractiveFile, O_RDONLY, 0640);
            if(dup2(newStdIn, fileno(stdin)) == -1){
                exit(EXIT_FAILURE);
            }
        } else {
            printf("NON INTERFATRIVE: %s\n", nonInteractiveFile);
        }
        if((getLineRet = getline(&cmdArgsMalloc, &size, stdin)) == -1){
            free(cmdArgsMalloc);
            fprintf(stdout, "\n");
            exit(0);
        }
        char * cmdToPass = malloc(getLineRet + 1);
        strcpy(cmdToPass, cmdArgsMalloc);

        char * token = strtok(cmdArgsMalloc, " ");
        char * first = token;
        if(strncmp(first,"exit", 4) == 0){
            int passingThis = CMD_EXIT;
            if(write(writeToServer, &passingThis, sizeof(passingThis)) == -1){
                fprintf(stderr, "Server not active!\n");
            }
            close(writeToServer);
            break;
        }
        else if(strncmp(first, "list", strlen("list")) == 0){
            int passingThis = CMD_LIST;
            if(write(writeToServer, &passingThis, sizeof(passingThis)) == -1){
                fprintf(stderr, "Server not active!\n");
            }

            int jobCount = 0;
            char * curListLine = malloc(100);
            if(curListLine == NULL){
                continue;
            }
            if(read(readFromServer, &jobCount, sizeof(jobCount)) == -1){
                fprintf(stderr, "Invalid read from server\n");
            }
            // int new = atoi(loopC);
            fprintf(stdout, "Count of All Jobs Submitted: %d\n", jobCount);
            if(jobCount == 0){
                fprintf(stdout, "You have not submitted any jobs.\n");
            } else {
                fprintf(stdout, "Job List:\n\n");
            }
            for(int j = 0; j < jobCount; j++){
                int curLineLen = 0;
                //read how many bytes is each line in the list
                if(read(readFromServer, &curLineLen, sizeof(curLineLen)) == -1){
                    fprintf(stderr, "Invalid read from server\n");
                }
                if(read(readFromServer, curListLine, curLineLen) == -1){
                    fprintf(stderr, "Invalid read from server\n");
                }
                fprintf(stdout, "%s\n", curListLine);
            }
        }
        else if(strncmp(first, "submit", strlen("submit")) == 0){ // Submit command
            int passingThis = CMD_SUBMIT;
            if(write(writeToServer, &passingThis, sizeof(passingThis)) == -1){
                fprintf(stderr, "Server not active!\n");
            }
            int cmdSize = getLineRet + 1;
            if(write(writeToServer, &cmdSize , sizeof(cmdSize)) == -1){ // Pass the size first so server can malloc
                fprintf(stderr, "Server not live\n");
            }
            if(write(writeToServer, cmdToPass, cmdSize) == -1){ // Pass the cmd itself after server mallocs
                fprintf(stderr, "Server not live\n");
            }
            int pollReadFromServer = poll(readFromPD, t, -1); // Wait for response
            if(pollReadFromServer == -1){
                fprintf(stdout, "poll Error\n");
            }
            char * ret = malloc(SUBMIT_MALLOC_SIZE); // Malloc a bit to get the response
            if(read(readFromServer, ret, SUBMIT_MALLOC_SIZE) == -1){
                fprintf(stderr, "Reading server response failed\n");
            }
            else {
                fprintf(stdout, "Submit Status: %s\n", ret); // Print out response
            }
            fflush(stdout);
        }
        else if(strncmp(first, "get", strlen("get")) == 0){
            int passingThis = CMD_GET;
            int passingID = atoi(cmdToPass);
            if(write(writeToServer, &passingThis, sizeof(passingThis)) == -1){
                fprintf(stderr, "Server not active!\n");
            }
            if(write(writeToServer, cmdToPass, strlen(cmdToPass)) == -1){
                fprintf(stderr, "Server not active!\n");
            }
            char txtFilePath[FILENAME_SIZE];
            char errFilePath[FILENAME_SIZE];
            if(sprintf(txtFilePath, "%d.txt", passingID) == -1){
                perror("fprintf error");
            }
            if(sprintf(errFilePath, "%d.err", passingID) == -1){
                perror("fprintf error");
            }
            fprintf(stdout, "OUTPUT FILE:\n");
            printOutFile(txtFilePath);
            fprintf(stdout, "ERROR LOG FILE:\n");
            printOutFile(errFilePath);
            fflush(stdout);
        }
        else if(strncmp(first, "suspend", strlen("suspend")) == 0){
            int passingThis = CMD_SUSPEND;
            if(write(writeToServer, &passingThis, sizeof(passingThis)) == -1){
                fprintf(stderr, "Server not active!\n");
            }
            if(write(writeToServer, cmdToPass, strlen(cmdToPass)) == -1){
                fprintf(stderr, "Server not active!\n");
            }
        }
        else if(strncmp(first, "kill", strlen("kill")) == 0){
            int passingThis = CMD_KILL;
            if(write(writeToServer, &passingThis, sizeof(passingThis)) == -1){
                fprintf(stderr, "Server not active!\n");
            }
            if(write(writeToServer, cmdToPass, strlen(cmdToPass)) == -1){
                fprintf(stderr, "Server not active!\n");
            }
        }
        else if(strncmp(first, "continue", strlen("continue")) == 0){
            int passingThis = CMD_CONT;
            if(write(writeToServer, &passingThis, sizeof(passingThis)) == -1){
                fprintf(stderr, "Server not active!\n");
            }
            if(write(writeToServer, cmdToPass, strlen(cmdToPass)) == -1){
                fprintf(stderr, "Server not active!\n");
            }
        }
        else {
            fprintf(stdout,"invalid cmd\n");
        }
        free(cmdArgsMalloc);
    }
    return 0;
}

void printOutFile(char* filePath){
    char *curFile = NULL;
    FILE *fp = fopen(filePath, "r");
    if (fp != NULL) {
        if (fseek(fp, 0L, SEEK_END) == 0) {
            long bufsize = ftell(fp);
            curFile = malloc(sizeof(char) * (bufsize + 1));
            fseek(fp, 0L, SEEK_SET);
            size_t newLen = fread(curFile, sizeof(char), bufsize, fp);
            if ( ferror( fp ) != 0 ) {
                fputs("Error reading file", stderr);
            } else {
                curFile[newLen++] = '\0'; /* Just to be safe. */
            }
            fprintf(stdout, "%s", curFile);
        }
        fclose(fp);
    }
    free(curFile);
}