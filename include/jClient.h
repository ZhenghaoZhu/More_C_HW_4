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

int jClientMain(char* nonInteractiveFile);
void printOutFile(char* filePath);
