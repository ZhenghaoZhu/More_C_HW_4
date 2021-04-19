#include <stdio.h>
#include <stdlib.h>
#include "ajs.h"

int main(){
    char option[2];
    printf("Server (0) or Client (1)?\n");
    if(fgets(option, 2, stdin) == NULL){
        printf("Error with fgets");
        return 1;
    }
    int optionInt = atoi(option);
    if(optionInt == 0){
        jServerMain();
    } else if(optionInt == 1) {
        jClientMain();
    } else {
        printf("Error, wrong option selected");
        return 1;
    }
    return 0;
}