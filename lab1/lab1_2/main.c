#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <file1> [file2...] <operation> [args]\n", argv[0]);
        return 1;
    }

    // Определение индекса операции
    int operation_index = -1;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "xor", 3) == 0) {
            int n = atoi(argv[i] + 3);
            if (n >= 2 && n <= 6) {
                operation_index = i;
                break;
            }
        } else if (strcmp(argv[i], "mask") == 0) {
            if (i + 1 < argc) {
                operation_index = i;
                break;
            }
        } else if (strncmp(argv[i], "copy", 4) == 0) {
            int n = atoi(argv[i] + 4);
            if (n >= 1) {
                operation_index = i;
                break;
            }
        } else if (strcmp(argv[i], "find") == 0) {
            if (i + 1 < argc) {
                operation_index = i;
                break;
            }
        }
    }

    if (operation_index == -1 || operation_index == 1) {
        fprintf(stderr, "Invalid operation or missing arguments\n");
        return 1;
    }

    int num_files = operation_index - 1;
    char **files = argv + 1;
    char *operation = argv[operation_index];

    if (strncmp(operation, "xor", 3) == 0) {
        int n = atoi(operation + 3);
        handle_xor(n, files, num_files);
    } else if (strcmp(operation, "mask") == 0) {
        if (operation_index + 1 >= argc) {
            fprintf(stderr, "Missing mask value\n");
            return 1;
        }
        handle_mask(argv[operation_index + 1], files, num_files);
    } else if (strncmp(operation, "copy", 4) == 0) {
        int n = atoi(operation + 4);
        handle_copy(n, files, num_files);
    } else if (strcmp(operation, "find") == 0) {
        if (operation_index + 1 >= argc) {
            fprintf(stderr, "Missing search string\n");
            return 1;
        }
        handle_find(argv[operation_index + 1], files, num_files);
    } else {
        fprintf(stderr, "Unknown operation\n");
        return 1;
    }

    return 0;
}