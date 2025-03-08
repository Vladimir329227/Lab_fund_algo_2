#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdint.h>

Error handle_xor(int N, char **files, int num_files) {
    int is_n2 = (N == 2);
    size_t block_size = is_n2 ? 0 : (1 << (N - 3));
    long file_size;
    unsigned char result, byte;
    size_t padded_size, rem;

    for (int i = 0; i < num_files; i++) {
        char *filename = files[i];
        FILE *f = fopen(filename, "rb");
        if (!f) {
            fprintf(stderr, "Error opening file %s\n", filename);
            continue;
        }

        fseek(f, 0, SEEK_END);
        file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        unsigned char *buffer = malloc(file_size);
        if (!buffer) {
            fclose(f);
            return ERROR_MEMORY;
        }

        if (fread(buffer, 1, file_size, f) != (long unsigned)file_size) {
            fprintf(stderr, "Error reading file %s\n", filename);
            free(buffer);
            fclose(f);
            continue;
        }
        fclose(f);

        if (is_n2) {
            result = 0;
            for (long j = 0; j < file_size; j++) {
                byte = buffer[j];
                result ^= (byte >> 4) & 0x0F;
                result ^= byte & 0x0F;
            }
            printf("%s: %02x\n", filename, result & 0x0F);
        } else {
            padded_size = file_size;
            rem = padded_size % block_size;
            if (rem != 0) {
                padded_size += block_size - rem;
            }

            unsigned char *padded_buffer = realloc(buffer, padded_size);
            if (!padded_buffer) {
                free(buffer);
                return ERROR_MEMORY;
            }
            memset(padded_buffer + file_size, 0, padded_size - file_size);

            unsigned char *result = calloc(block_size, 1);
            if (!result) {
                free(padded_buffer);
                return ERROR_MEMORY;
            }

            for (size_t j = 0; j < padded_size; j += block_size)
                for (size_t k = 0; k < block_size; k++)
                    result[k] ^= padded_buffer[j + k];

            printf("%s: ", filename);
            for (size_t k = 0; k < block_size; k++)
                printf("%02x", result[k]);
            printf("\n");

            free(result);
            free(padded_buffer);
            continue; 
        }

        free(buffer);
    }
    return OK;
}

Error handle_mask(const char *mask_str, char **files, int num_files) {
    char *endptr;
    unsigned long mask = strtoul(mask_str, &endptr, 16);

    if (endptr == mask_str || *endptr != '\0') {
        fprintf(stderr, "Invalid mask value: %s\n", mask_str);
        return ERROR_INPUT;
    }

    uint32_t mask_value = (uint32_t)mask;
    long file_size;
    uint32_t count = 0, num;
    for (int i = 0; i < num_files; i++) {
        char *filename = files[i];
        FILE *f = fopen(filename, "rb");
        if (!f) {
            fprintf(stderr, "Error opening file %s\n", filename);
            continue;
        }

        fseek(f, 0, SEEK_END);
        file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        unsigned char *buffer = malloc(file_size);
        if (!buffer) {
            fclose(f);
            return ERROR_MEMORY;
        }

        if (fread(buffer, 1, file_size, f) != (long unsigned)file_size) {
            fprintf(stderr, "Error reading file %s\n", filename);
            free(buffer);
            fclose(f);
            continue;
        }
        fclose(f);

        count = 0;
        for (size_t j = 0; j <= (long unsigned)file_size - 4; j += 4) {
            num = (buffer[j] << 24) | (buffer[j+1] << 16) | (buffer[j+2] << 8) | buffer[j+3];
            if ((num & mask_value) == mask_value) {
                count++;
            }
        }

        printf("%s: %u\n", filename, count);
        free(buffer);
    }
    return OK;
}

Error handle_copy(int N, char **files, int num_files) {

    for (int i = 0; i < num_files; i++) {
        char *src_name = files[i];
        for (int copy_num = 1; copy_num <= N; copy_num++) {
            pid_t pid = fork();
            if (pid == 0) {
                char dest_name[PATH_MAX];
                snprintf(dest_name, PATH_MAX, "%s%d", src_name, copy_num);

                FILE *src = fopen(src_name, "rb");
                if (!src) {
                    exit(EXIT_FAILURE);
                }

                FILE *dest = fopen(dest_name, "wb");
                if (!dest) {
                    fclose(src);
                    exit(EXIT_FAILURE);
                }

                unsigned char buffer[BUFFER_SIZE];
                size_t bytes_read;
                while ((bytes_read = fread(buffer, 1, sizeof(buffer), src))) {
                    fwrite(buffer, 1, bytes_read, dest);
                }

                fclose(src);
                fclose(dest);
                exit(EXIT_SUCCESS);
            } else if (pid < 0) {
                perror("fork");
            }
        }
    }

    for (int i = 0; i < num_files * N; i++)
        wait(NULL);
    
    return OK;
}


Error handle_find(const char *search_str, char **files, int num_files) {
    int found = 0;
    for (int i = 0; i < num_files; i++) {
        char *filename = files[i];
        pid_t pid = fork();
        if (pid == 0) { 
            FILE *f = fopen(filename, "r");
            if (!f) {
                exit(EXIT_FAILURE);
            }

            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fseek(f, 0, SEEK_SET);

            char *content = malloc(size + 1);
            if (!content) {
                fclose(f);
                exit(EXIT_FAILURE);
            }

            size_t bytes_read = fread(content, 1, size, f);
            content[bytes_read] = '\0';
            fclose(f);

            int result = (strstr(content, search_str) != NULL);
            free(content);
            exit(result ? EXIT_SUCCESS : EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
            continue;
        } else {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                printf("%s\n", filename);
                found = 1;
            }
        }
    }

    if (!found)
        printf("Not find.\n");
    
    return OK;
}