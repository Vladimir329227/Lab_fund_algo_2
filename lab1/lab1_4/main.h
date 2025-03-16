#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#ifndef MAIN_H_FLAG
#define MAIN_H_FLAG


typedef enum {
    OK,
    ERROR,
    ERROR_INPUT,
    ERROR_FILE,
    ERROR_MEMORY
} Error;

#define MAX_CMD_LEN 256
#define MSG_TYPE 1

typedef struct {
    long mtype;
    int user_id;
    char command[MAX_CMD_LEN];
} Message;

#endif