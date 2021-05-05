#ifndef SERVER_HEADER
#define SERVER_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>

typedef struct job_status{
    pid_t pid;
    time_t timeStart;
    time_t timeEnd;
    clock_t clockStart;
    clock_t clockEnd;
    int timeDifference;
    double clockDifference;
    int id;
    int status; //0 = finished, 1 = stopped, 2 = running
    struct job_status *next;
} JOB_STATUS;

#define listSize 1
struct job_status list[listSize];

#endif