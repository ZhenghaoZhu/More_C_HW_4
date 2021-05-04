// #define EXIT_SUCCESS 0
// #define EXIT_FAILURE 1

int jServerMain(int max);
int jClientMain();

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <stdio.h>

#define B_SIZE (PIPE_BUF/2)

struct message {
   char fifo_name[B_SIZE];
   char cmd_line[B_SIZE];
};
