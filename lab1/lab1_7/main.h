#include <stdbool.h>
#include <stdio.h>

#ifndef MAIN_H_FLAG
#define MAIN_H_FLAG


typedef enum {
    OK,
    ERROR,
    ERROR_INPUT,
    ERROR_FILE,
    ERROR_MEMORY
} Error;

#endif