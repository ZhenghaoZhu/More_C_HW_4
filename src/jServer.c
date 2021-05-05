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
#include <sys/select.h>
#include <poll.h>

#include "jServer.h"
#include "ajs.h"

char* STATUS_ARRAY[3] = {"FINISHED", "STOPPED", "RUNNING"};

static int listLength = 0;
static int curNumJobs = 0;

void sig_child_handler(){
    pid_t childPID;
    clock_t chldClockEnd;
    time_t chldTimeEnd;
    int stat;
    if((childPID = waitpid(-1, &stat, WNOHANG)) != -1){
        chldClockEnd = clock(); // Get times before searching linked list to be accurate
        chldTimeEnd = time(NULL); // As searching the LL might take time
        struct job_status* ptr = &head[0];
        ptr = ptr->next;
        while(ptr != NULL){
            if(ptr->pid == childPID){
                ptr->clockEnd = chldClockEnd;
                ptr->timeEnd = chldTimeEnd;
                ptr->status = JOB_FINISHED;
                curNumJobs -= 1; // Decrease number of running jobs
                break;
            }
            ptr = ptr->next;
        }
    }
}

void initializeJobList(){
    head[0].id = -1;
    head[0].next = NULL;
}

void addJobToLL(struct job_status* newNode){
    newNode->next = head[0].next; // Add new node to the start
    head[0].next = newNode;
    listLength += 1;
}

int jServerMain(int maxConcurrentJobs){
    fprintf(stdout, "jServer Waiting for Connections!\n");
    if(head[0].id != -1){
        initializeJobList();
    }
    if(signal(SIGCHLD, sig_child_handler) == SIG_ERR){
        fprintf(stdout, "Handler not set up correctly\n");
    }

    int readFromClient = open(C_TO_S, O_RDONLY);
    if(readFromClient == -1){
        perror("Unable to open C_TO_S");
        exit(EXIT_FAILURE);
    }
    int writeToClient = open(S_TO_C, O_WRONLY);
    if(writeToClient == -1){
        perror("Unable to open S_TO_C");
        exit(EXIT_FAILURE);
    }
    struct pollfd * readFromClientPD = malloc(sizeof(struct pollfd));
    if(readFromClientPD == NULL){
        perror("Invalid malloc");
        exit(EXIT_FAILURE);
    }
    readFromClientPD->fd = readFromClient;
    readFromClientPD->events = POLLIN;
    nfds_t elemCnt = 1;
    

    int id = 0;
    fprintf(stdout, "jServer Running!\n\n");

    while(true){
        int checkPoll = poll(readFromClientPD, elemCnt, -1);
        if(readFromClientPD->revents != 1){ // Reset pipe when client exits
            close(readFromClient);
            readFromClient = open(C_TO_S, O_RDONLY);
            readFromClientPD->fd = readFromClient;
            readFromClientPD->events = POLLIN;
            continue;
        }

        char* readMalloc = malloc(READ_MALLOC_SIZE);

        if(readMalloc == NULL){
            perror("readMalloc error");
            exit(EXIT_FAILURE);
        }
        int container = 0;
        if(read(readFromClient, &container, sizeof(int)) == -1){
            fprintf(stderr, "Can't read from client \n");
            continue;
        }

        if(container == CMD_GET){
            fprintf(stdout, "Received [get] command\n");
            char * givenCmd = calloc(sizeof(char), READ_MALLOC_SIZE);
            if(read(readFromClient, givenCmd, READ_MALLOC_SIZE) == -1){
                fprintf(stderr, "Reading from client failed.\n");
            }
            char* skipGet = givenCmd + strlen("get");
            int passedID = atoi(skipGet);
            char fileName[FILENAME_SIZE];
            sprintf(fileName, "%d.txt", passedID);
            fprintf(stdout, "Getting data from: %d.txt and %d.err\n", passedID, passedID);
            fflush(stdout);
        }
        else if(container == CMD_SUSPEND){
            fprintf(stdout, "Received [suspend] command\n");
            char * givenCmd = calloc(sizeof(char), READ_MALLOC_SIZE);
            if(read(readFromClient, givenCmd, READ_MALLOC_SIZE) == -1){
                fprintf(stderr, "Reading from client failed.\n");
            }
            char* skipGet = givenCmd + strlen("suspend");
            int passedID = atoi(skipGet);
            struct job_status* ptr = &head[0];
            ptr = ptr->next;
            while(ptr != NULL){
                if(ptr->id == passedID){
                    kill(ptr->pid, SIGSTOP);
                    ptr->status = JOB_STOPPED;
                    break;
                }
                ptr = ptr->next;
            }
            fflush(stdout);
        }
        else if(container == CMD_KILL){
            fprintf(stdout, "Received [kill] command\n");
            char * givenCmd = calloc(sizeof(char), READ_MALLOC_SIZE);
            if(read(readFromClient, givenCmd, READ_MALLOC_SIZE) == -1){
                fprintf(stderr, "Reading from client failed.\n");
            }
            char* skipGet = givenCmd + strlen("kill");
            int passedID = atoi(skipGet);
            struct job_status* ptr = &head[0];
            ptr = ptr->next;
            while(ptr != NULL){
                if(ptr->id == passedID){
                    kill(ptr->pid, SIGKILL);
                    ptr->status = JOB_STOPPED;
                    ptr->clockEnd = clock();
                    ptr->timeEnd = time(NULL);
                    break;
                }
                ptr = ptr->next;
            }
            fflush(stdout);
        }
        else if(container == CMD_CONT){
            fprintf(stdout, "Received [continue] command\n");
            char * givenCmd = calloc(sizeof(char), READ_MALLOC_SIZE);
            if(read(readFromClient, givenCmd, READ_MALLOC_SIZE) == -1){
                fprintf(stderr, "Reading from client failed.\n");
            }
            char* skipGet = givenCmd + strlen("kill");
            int passedID = atoi(skipGet);
            struct job_status* ptr = &head[0];
            ptr = ptr->next;
            while(ptr != NULL){
                if(ptr->id == passedID){
                    kill(ptr->pid, SIGCONT);
                    ptr->status = JOB_RUNNING;
                    break;
                }
                ptr = ptr->next;
            }
            fflush(stdout);
        }
        else if(container == CMD_EXIT){
            fprintf(stdout, "Received [exit] command\n");
            fflush(stdout);
            free(readMalloc);
            continue;
        } else if(container == CMD_LIST){//list all jobs
            fprintf(stdout, "Received [list] command\n");
            if(write(writeToClient, &listLength, sizeof(listLength)) == -1){
                perror("Unable to write to client");
            }
            struct job_status * ptr = &head[0];
            ptr = ptr->next;
            char* curListLine = malloc(LIST_LINE_MALLOC);
            if(curListLine == NULL){
                continue;
            }
            while(ptr != NULL && ptr != &head[0]){
                int size = 0;
                if(ptr->status == JOB_RUNNING){
                    size = sprintf(curListLine,"ID: %d, PID: %d, STATUS: %s\n", ptr->id, ptr->pid, STATUS_ARRAY[ptr->status]);
                } else {
                    ptr->timeDifference = (ptr->timeEnd - ptr->timeStart);
                    ptr->clockDifference = (ptr->clockEnd - ptr->clockStart);
                    size = sprintf(curListLine,"ID: %d, Process ID: %d, Status: %s, CPU Time: %4f, Normal Time: %i\n", ptr->id, ptr->pid, STATUS_ARRAY[ptr->status], ptr->clockDifference, ptr->timeDifference);
                }
                if(write(writeToClient, &size, sizeof(size)) == -1){
                    perror("Unable to write to client");
                }
                if(write(writeToClient, curListLine, size) == -1){
                    perror("Invalid write to client");
                }
                ptr = ptr->next;
            }
            free(curListLine);
            free(readMalloc);
            readMalloc = NULL;
            fflush(stdout);
        } 
        else if(container == CMD_SUBMIT){
            fprintf(stdout, "Received [submit] command\n");
            int cmdLen = 0;
            if(read(readFromClient, &cmdLen , sizeof(cmdLen)) == -1){
                fprintf(stderr, "Reading from client failed.\n");
            }
            char* givenCmd = malloc(cmdLen);
            if(read(readFromClient, givenCmd, cmdLen) == -1){
                fprintf(stderr, "Reading from client failed.\n");
            }
            fprintf(stdout, "Command Received: %s\n", givenCmd);
            char * skipSubmit = givenCmd + strlen("submit"); // Jumping "submit"
            struct job_status * curJob = malloc(sizeof(struct job_status));
            curJob->clockStart = clock();
            curJob->clockDifference = -1.0;
            curJob->timeStart = time(NULL);
            char jobOutput[FILENAME_SIZE];
            char jobErr[FILENAME_SIZE];
            if(sprintf(jobOutput, "%d.txt", id) == -1 || sprintf(jobErr, "%d.err", id) == -1 ){
                fprintf(stderr, "sprintf failure\n");
            }
            else{
                id += 1;
            }
            FILE* curJobFile = fopen(jobOutput, "w");
            FILE* curJobErr = fopen(jobErr, "w");
            if(curJobFile == NULL || curJobErr == NULL){
                perror("Unable to open job file(s)");
                free(readMalloc);
                readMalloc = NULL;
                continue;
            }

            pid_t pid = fork();

            if(pid == 0){
                close(STDOUT_FILENO); 
                if(dup(fileno(curJobFile)) == -1){ // dup respective out file
                    perror("stdout dup failed");
                    exit(EXIT_FAILURE);
                }

                close(STDERR_FILENO);
                if(dup(fileno(curJobErr)) == -1){ // dup respective err file
                    perror("stderr dup failed");
                    exit(EXIT_FAILURE);
                }

                if(system(skipSubmit) == -1){
                    fprintf(stderr, "System call with given cmd failed.\n");
                    exit(EXIT_FAILURE);
                }
                curNumJobs += 1;
                exit(EXIT_SUCCESS);
            } else {
                int stat;
                int prevID = id - 1;
                char idToStr[12];
                sprintf(idToStr, "%d", prevID);
                char * clientSubmitRet = calloc(sizeof(char), SUBMIT_MALLOC_SIZE);
                strcat(clientSubmitRet, "Job Queued with ID: ");
                strcat(clientSubmitRet, idToStr);
                if(write(writeToClient, clientSubmitRet, strlen(clientSubmitRet)) == -1){
                    fprintf(stderr, "Read no good!\n");
                }
                free(clientSubmitRet);
            }

            if(pid != 0){
                curJob->id = id - 1;
                curJob->pid = pid;
                curJob->status = JOB_RUNNING;
                addJobToLL(curJob);
            }
            fflush(stdout);
            free(readMalloc);
            readMalloc = NULL;
        } 
        else {
            fprintf(stdout, "Invalid 1st byte!\n");
            fflush(stdout);
            free(readMalloc);
            readMalloc = NULL;
        }
    }
    return 0;
}