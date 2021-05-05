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
    clock_t cpuStart;
    clock_t cpuEnd;
    double cpuTime;
    time_t timeStart;
    time_t timeEnd;
    pid_t pid;
    int id;
    int status; //0 = finished, 1 = stopped, 2 = running
    struct job_status *next;
} JOB_STATUS;

#define listSize 1
struct job_status list[listSize];

int server(int max);

#endif