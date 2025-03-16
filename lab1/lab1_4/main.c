#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

typedef struct {
    int wolf;
    int goat; 
    int cabbage;
} Shore;

typedef struct {
    int peasant;
    char boat[10];
    Shore left;
    Shore right;
    bool game_over;
    bool won;
} GameState;

typedef struct GameStateNode {
    int user_id;
    GameState state;
    struct GameStateNode* next;
} GameStateNode;

volatile sig_atomic_t stop_flag = 0;

void print_state(GameState *state) {
    printf("\nCurrent state:\n");
    printf("Left  shore: wolf=%d, goat=%d, cabbage=%d\n", 
           state->left.wolf, state->left.goat, state->left.cabbage);
    printf("Right shore: wolf=%d, goat=%d, cabbage=%d\n", 
           state->right.wolf, state->right.goat, state->right.cabbage);
    printf("Boat contains: [%s]\n", state->boat[0] ? state->boat : "empty");
    printf("Peasant position: %s\n\n", 
           state->peasant ? "RIGHT" : "LEFT");
}

void handle_signal(__attribute__((unused)) int sig) {
    stop_flag = 1;
}

GameStateNode* find_user(GameStateNode* head, int user_id) {
    while (head) {
        if (head->user_id == user_id) return head;
        head = head->next;
    }
    return NULL;
}

void initialize_state(GameState* state) {
    state->peasant = 0;
    strcpy(state->boat, "");
    state->left.wolf = state->left.goat = state->left.cabbage = 1;
    state->right.wolf = state->right.goat = state->right.cabbage = 0;
    state->game_over = false;
    state->won = false;
}

bool check_lose(GameState* state, int shore) {
    if (!state)
        return true;
    int wolf = shore ? state->left.wolf : state->right.wolf;
    int goat = shore ? state->left.goat : state->right.goat;
    int cabbage = shore ? state->left.cabbage : state->right.cabbage;
    return (wolf && goat) || (goat && cabbage);
}

bool check_win(GameState* state) {
    if (!state)
        return true;
    return state->right.wolf && state->right.goat && state->right.cabbage;
}

bool process_command(GameState* state, const char* cmd) {
    if (!state || !cmd)
        return false;    

    if (state->game_over) {
        printf("Game over\n");
        return false;
    }

    if (strncmp(cmd, "take ", 5) == 0) {
        char obj[20];
        if (sscanf(cmd+5, "%19s", obj) != 1) {
            printf("Invalid take command\n");
            return false;
        }

        if (strlen(state->boat) != 0) {
            printf("Boat already contains: %s\n", state->boat);
            return false;
        }

        Shore* current_shore = state->peasant ? &state->right : &state->left;

        if (strcmp(obj, "wolf") == 0) {
            if (!current_shore->wolf) {
                printf("No wolf\n");
                return false;
            }
            current_shore->wolf = 0;
        }
        else if (strcmp(obj, "goat") == 0) {
            if (!current_shore->goat) {
                printf("No goat\n");
                return false;
            }
            current_shore->goat = 0;
        }
        else if (strcmp(obj, "cabbage") == 0) {
            if (!current_shore->cabbage) {
                printf("No cabbage\n");
                return false;
            }
            current_shore->cabbage = 0;
        }
        else {
            printf("Invalid object: %s\n", obj);
            return false;
        }

        strcpy(state->boat, obj);
        printf("Taken %s successfully %li\n", state->boat, strlen(state->boat));

        return true;
    }
    else if (strncmp(cmd, "put", 3) == 0) {
        if (strlen(state->boat) == 0) {
            printf("Boat is empty\n");
            return false;
        }

        Shore* shore = state->peasant ? &state->right : &state->left;

        if (!strcmp(state->boat, "wolf")) shore->wolf = 1;
        else if (!strcmp(state->boat, "goat")) shore->goat = 1;
        else if (!strcmp(state->boat, "cabbage")) shore->cabbage = 1;

        strcpy(state->boat, "");

        if (check_win(state)) {
            state->game_over = state->won = true;
            printf("All objects safely transported\n");
        }

        return true;
    }
    else if (strncmp(cmd, "move", 4) == 0) {
        int prev_shore = state->peasant;
        state->peasant = !(state->peasant);        

        if (check_lose(state, prev_shore)) {
            printf("Dangerous combination left on %s shore\n", 
                   prev_shore ? "right" : "left");
            state->game_over = true;
            return false;
        }
        
        return true;
    }
    
    printf("Unknown command: %s\n", cmd);
    return false;
}

void free_user(GameStateNode** head, int user_id) {
    if(!head)
        return;
    GameStateNode *current = *head, *prev = NULL;
    while (current && current->user_id != user_id) {
        prev = current;
        current = current->next;
    }
    if (!current) return;
    if (prev) prev->next = current->next;
    else *head = current->next;
    free(current);
}

void cleanup(GameStateNode** head) {
    if(!head)
        return;
    GameStateNode *current = *head;
    while (current) {
        GameStateNode *next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    key_t key = ftok("msgqueue", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    GameStateNode* states = NULL;

    while (!stop_flag) {
        Message msg;
        ssize_t received = msgrcv(msgid, &msg, sizeof(Message)-sizeof(long), MSG_TYPE, IPC_NOWAIT);
        
        if (received == -1) {
            if (errno == ENOMSG) {
                sleep(1);
                continue;
            }
            perror("msgrcv");
            break;
        }

        GameStateNode* user = find_user(states, msg.user_id);
        if (!user) {
            user = malloc(sizeof(GameStateNode));
            user->user_id = msg.user_id;
            initialize_state(&user->state);
            user->next = states;
            states = user;
            printf("\n=== New user %d created ===\n", msg.user_id);
            print_state(&user->state);
        }

        printf("\nUser %d executing: %s\n", msg.user_id, msg.command);
        bool res = process_command(&user->state, msg.command);
        printf("Result: %s\n", res ? "OK" : "FAIL");
        
        if (user->state.won) {
            printf("=== USER %d WON! ===\n", msg.user_id);
            free_user(&states, msg.user_id);
        }
        else if (user->state.game_over) {
            printf("=== USER %d LOST! ===\n", msg.user_id);
            free_user(&states, msg.user_id);
        }
    }

    cleanup(&states);
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}