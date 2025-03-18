#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "main.h"

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        return ERROR;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("invalid address");
        return ERROR;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        return ERROR;
    }

    printf("Enter directory paths (empty line to finish):\n");
    char *line = NULL;
    size_t len = 0;
    ssize_t bytes_read;

    while ((bytes_read = getline(&line, &len, stdin)) != -1) {
        if (line[0] == '\n') {
            write(sock, "\n", 1);
            break;
        }
        line[strcspn(line, "\n")] = 0;
        dprintf(sock, "%s\n", line);
    }

    free(line);

    char buffer[4096] = {0};
    ssize_t received = read(sock, buffer, sizeof(buffer));
    if (received > 0) {
        printf("Server response:\n%s", buffer);
    }

    close(sock);
    return 0;
}