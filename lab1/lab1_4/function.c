#include "main.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <command_file>\n", argv[0]);
        return ERROR_INPUT;
    }

    srand(time(NULL) ^ getpid());
    int user_id = rand();

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("fopen");
        return ERROR;
    }

    key_t key = ftok("msgqueue", 65);
    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("msgget");
        return ERROR;
    }

    char line[MAX_CMD_LEN];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        Message msg = { .mtype = MSG_TYPE, .user_id = user_id };
        strncpy(msg.command, line, MAX_CMD_LEN);
        
        if (msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0) == -1)
            perror("msgsnd");
    }

    fclose(file);
    return OK;
}