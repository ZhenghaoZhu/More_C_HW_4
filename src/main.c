#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "jClient.h"
#include "jServer.h"
#include "ajs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

int main(int argc, char *argv[]){
    if(argc != 3){
        fprintf(stderr, "Invalid # of params! \n");   
    }
    else if(strcmp(argv[1],"1") == 0){
        if(access( "tmp/.special", F_OK ) == 0 ) {
        } else {
            mkfifo("tmp/.special", 0666);
        }
        if(access( "tmp/.sTOc", F_OK ) == 0 ) {
        } else {
            mkfifo("tmp/.sTOc", 0666);
        }
        jClientMain();
    }
    else if(strcmp(argv[1],"2") == 0){
        int max = atoi(argv[2]);
        if(max != 0){
            if(access( "tmp/special", F_OK ) == 0 ) {
                fprintf(stdout, "Special exists\n");
            } else {
                mkfifo("tmp/special", 0666);
            }
            if(access( "tmp/.sTOc", F_OK ) == 0 ) {
            } else {
                mkfifo("tmp/.sTOc", 0666);
            }
            jServerMain(max);
        }
        else {
            fprintf(stdout, "Invalid max size\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        fprintf(stderr, "Not client or server code! \n");
    }
    printf("yes\n");
    return 0;   
}