#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "main.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_RESPONSE 4096
#define FULL_PATH_MAX 4096

void *my_memmem(const void *haystack, size_t haystacklen,
               const void *needle, size_t needlelen) {
    if (needlelen == 0) return (void*)haystack;
    if (haystacklen < needlelen || !haystack || !needle) return NULL;

    const char *h = haystack;
    const char *n = needle;
    for (size_t i = 0; i <= haystacklen - needlelen; i++)
        if (memcmp(&h[i], n, needlelen) == 0)
            return (void*)&h[i];

    return NULL;
}

char* get_dir_contents(const char *path) {
    if (!path)
        return NULL;

    DIR *dir = opendir(path);
    if (!dir) return NULL;

    char *result = malloc(MAX_RESPONSE);
    result[0] = '\0';
    struct dirent *entry;
    struct stat st;
    char full_path[FULL_PATH_MAX];  

    while ((entry = readdir(dir)) != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (stat(full_path, &st) == -1) continue;
        if (S_ISREG(st.st_mode)) {
            strncat(result, entry->d_name, MAX_RESPONSE - strlen(result) - 1);
            strncat(result, "\n", MAX_RESPONSE - strlen(result) - 1);
        }
    }
    closedir(dir);
    return result;
}

char* process_path(const char *path) {
    if (!path)
        return NULL;

    struct stat st;
    if (stat(path, &st) == -1)
        return NULL;

    if (S_ISDIR(st.st_mode)) 
        return get_dir_contents(path);

    return NULL;
}

char* read_until_delimiter(int sockfd, const char *delim) {
    if (!delim)
        return NULL;

    static char buffer[BUFFER_SIZE];
    static size_t buf_len = 0;
    char *line = NULL;
    size_t line_len = 0;
    size_t delim_len = strlen(delim);

    while (1) {
        char *ptr = my_memmem(buffer, buf_len, delim, delim_len);
        if (ptr) {
            size_t chunk_len = ptr - buffer;
            line = realloc(line, line_len + chunk_len + 1);
            memcpy(line + line_len, buffer, chunk_len);
            line_len += chunk_len;
            line[line_len] = '\0';

            memmove(buffer, ptr + delim_len, buf_len - chunk_len - delim_len);
            buf_len -= chunk_len + delim_len;
            return line;
        }

        if (line) {
            line = realloc(line, line_len + buf_len + 1);
            memcpy(line + line_len, buffer, buf_len);
            line_len += buf_len;
            line[line_len] = '\0';
            buf_len = 0;
        }

        ssize_t n = read(sockfd, buffer, BUFFER_SIZE);
        if (n <= 0) {
            if (line_len > 0) return line;
            free(line);
            return NULL;
        }
        buf_len = n;
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        return ERROR;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        return ERROR;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return ERROR;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return ERROR;
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        char response[MAX_RESPONSE] = {0};
        char *path;

        while ((path = read_until_delimiter(new_socket, "\n")) != NULL) {
            if (strlen(path) == 0) {
                free(path);
                break;
            }

            char *contents = process_path(path);
            if (contents) {
                strcat(response, "Directory: ");
                strcat(response, path);
                strcat(response, "\nFiles:\n");
                strcat(response, contents);
                free(contents);
            } else {
                strcat(response, "Invalid path: ");
                strcat(response, path);
                strcat(response, "\n");
            }
            free(path);
        }

        send(new_socket, response, strlen(response), 0);
        close(new_socket);
    }

    close(server_fd);
    return 0;
}