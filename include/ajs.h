#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define C_TO_S "tmp/.cToS"
#define S_TO_C "tmp/.sToC"

#define JOB_RUNNING  2
#define JOB_STOPPED  1
#define JOB_FINISHED 0   

#define CMD_EXIT    1
#define CMD_LIST    2
#define CMD_SUBMIT  3
#define CMD_GET     4
#define CMD_SUSPEND 5
#define CMD_KILL    6
#define CMD_CONT    7

#define LIST_LINE_MALLOC 200
#define READ_MALLOC_SIZE   50
#define SUBMIT_MALLOC_SIZE 30
#define FILENAME_SIZE      30

int jServerMain(int max);
int jClientMain();

