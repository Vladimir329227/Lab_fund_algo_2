#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include "main.h"

bool is_unique_user(User user, Users users) {
    for (int i = 0; i < users.count; ++i) {
        if (strcmp(user.login, users.users[i].login) == 0) {
            return false;
        }
    }
    return true;
}

bool is_valid_user(User user) {
    if (strlen(user.login) > 6 || strlen(user.login) == 0 || user.pin < 0 || user.pin > 100000 || user.cur_token < 0)
        return false;       
    
    bool is_digit = false, is_alpha = false;
    for (size_t i = 0; i < strlen(user.login); i++){
        if (isdigit(user.login[i]) != 0)
            is_digit = true;
        else if (isalpha(user.login[i]) != 0)
            is_alpha = true;
        else
            return false;
    }
    
    if (!is_digit || !is_alpha)
        return false;

    return true;
}

Error add_user(Users *users, User user) {
    if (users->users == NULL)
        return ERROR_INPUT;

    Users* rez;
    if (users->count >= users->max_count) {
        users->max_count += REALLOC_STEP;
        rez = realloc(users->users, users->max_count * sizeof(User));
        if (rez == NULL){
            free(users->users);
            return ERROR_MEMORY;
        }
    }

    users->users[users->count++] = user;
    return OK;
}

void print_user(User user) {
    printf("%s\t%d\t%d\t%d\n", user.login, user.pin, user.max_token, user.cur_token);
}

void print_users(Users users) {
    for (int i = 0; i < users.count; ++i)
        print_user(users.users[i]);
}

Error read_file_users(FILE *fp, Users* users) {
    if (!fp)
        return ERROR_FILE;
    User user;
    while (fscanf(fp, "%s | %i | %i | %i\n", user.login, &user.pin, &user.max_token, &user.cur_token) == 4)
        if (is_valid_user(user))
            add_user(users, user);
        
     
    return OK;
}

Error save_file_users(FILE *fp, Users users) {
    if (!fp)
        return ERROR_FILE;
    for (int i = 0; i < users.count; ++i)
        fprintf(fp, "%s | %i | %i | %i\n", users.users[i].login, users.users[i].pin, users.users[i].max_token, users.users[i].cur_token);
     
    return OK;
}

Error init_users(Users* users) {
    if (!users)
        return ERROR_INPUT;
    
    users->count = 0;
    users->max_count = 5;
    users->users = (User*)malloc(sizeof(User) * 5);
    
    if(!users->users)
        return ERROR_MEMORY;

    return OK;
}

Error free_users(Users* users) {
    if (!users)
        return ERROR_INPUT;
    free(users->users);
    return OK;
}

bool login(Users users, User* user) {
    for (int i = 0; i < users.count; i++)
        if (users.users[i].pin == user->pin && strcmp(users.users[i].login, user->login) == 0){
            user->cur_token = 0;
            user->max_token = users.users[i].max_token;
            return true;
        }
    
    return false;
}

bool set_tocken(char* login, int token, Users users) {
    for (int i = 0; i < users.count; i++)
        if (strcmp(users.users[i].login, login) == 0){
            users.users[i].max_token = token;
            return true;
        }
    return false;
}