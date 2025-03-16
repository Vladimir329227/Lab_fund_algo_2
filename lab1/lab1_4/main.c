#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
    int peasant;
    char boat[10];
    struct { int wolf, goat, cabbage; } left, right;
    bool game_over;
    bool won;
} GameState;

typedef struct GameStateNode {
    int user_id;
    GameState state;
    struct GameStateNode* next;
} GameStateNode;

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
    int wolf = shore ? state->right.wolf : state->left.wolf;
    int goat = shore ? state->right.goat : state->left.goat;
    int cabbage = shore ? state->right.cabbage : state->left.cabbage;
    return (wolf && goat) || (goat && cabbage);
}

bool check_win(GameState* state) {
    return state->right.wolf && state->right.goat && state->right.cabbage;
}

bool process_command(GameState* state, const char* cmd) {
    if (state->game_over) return false;

    if (strncmp(cmd, "take ", 5) == 0) {
        char obj[10];
        if (sscanf(cmd+5, "%9s", obj) != 1) return false;
        if (strcmp(obj, "wolf") && strcmp(obj, "goat") && strcmp(obj, "cabbage")) return false;
        if (strlen(state->boat) != 0) return false;

        int* ptr = NULL;
        if (state->peasant == 0) {
            if (!strcmp(obj, "wolf") && state->left.wolf) ptr = &state->left.wolf;
            else if (!strcmp(obj, "goat") && state->left.goat) ptr = &state->left.goat;
            else if (!strcmp(obj, "cabbage") && state->left.cabbage) ptr = &state->left.cabbage;
        } else {
            if (!strcmp(obj, "wolf") && state->right.wolf) ptr = &state->right.wolf;
            else if (!strcmp(obj, "goat") && state->right.goat) ptr = &state->right.goat;
            else if (!strcmp(obj, "cabbage") && state->right.cabbage) ptr = &state->right.cabbage;
        }
        if (!ptr) return false;
        *ptr = 0;
        strcpy(state->boat, obj);
        return true;
    } else if (!strcmp(cmd, "put")) {
        if (strlen(state->boat) == 0) return false;
        int* ptr = NULL;
        if (state->peasant == 0) {
            if (!strcmp(state->boat, "wolf")) ptr = &state->left.wolf;
            else if (!strcmp(state->boat, "goat")) ptr = &state->left.goat;
            else if (!strcmp(state->boat, "cabbage")) ptr = &state->left.cabbage;
        } else {
            if (!strcmp(state->boat, "wolf")) ptr = &state->right.wolf;
            else if (!strcmp(state->boat, "goat")) ptr = &state->right.goat;
            else if (!strcmp(state->boat, "cabbage")) ptr = &state->right.cabbage;
        }
        if (!ptr) return false;
        *ptr = 1;
        strcpy(state->boat, "");
        return true;
    } else if (!strcmp(cmd, "move")) {
        int prev_shore = state->peasant;
        state->peasant = !state->peasant;
        if (check_lose(state, prev_shore)) {
            state->game_over = true;
            return false;
        }
        if (check_win(state)) {
            state->game_over = state->won = true;
            return true;
        }
        return true;
    }
    return false;
}

int main() {
    key_t key = ftok("msgqueue", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    GameStateNode* states = NULL;
    Message msg;

    while (1) {
        if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), MSG_TYPE, 0) == -1) {
            perror("msgrcv");
            continue;
        }

        GameStateNode* user = find_user(states, msg.user_id);
        if (!user) {
            user = malloc(sizeof(GameStateNode));
            if (!user)
                return ERROR_MEMORY;
            
            user->user_id = msg.user_id;
            initialize_state(&user->state);
            user->next = states;
            states = user;
        }

        if (user->state.game_over) {
            printf("User %d: Game already over\n", msg.user_id);
            free(user);
            continue;
        }

        printf("User %d: %s\n", msg.user_id, msg.command);
        bool res = process_command(&user->state, msg.command);
        printf("Result: %s\n", res ? "OK" : "FAIL");
        
        if (user->state.won) 
            printf("User %d won!\n", msg.user_id);
        else if (user->state.game_over) 
            printf("User %d lost!\n", msg.user_id);
        
        free(user);
    }

    msgctl(msgid, IPC_RMID, NULL);
    free(states);
    return 0;
}