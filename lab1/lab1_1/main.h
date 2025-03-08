#include <stdbool.h>
#include <stdio.h>

#ifndef MAIN_H_FLAG
#define MAIN_H_FLAG

#define REALLOC_STEP 5

typedef enum {
    OK,
    ERROR,
    ERROR_INPUT,
    ERROR_FILE,
    ERROR_MEMORY
} Error;

typedef struct {
    char login[6];
    int pin;
    int max_token;
    int cur_token;
} User;

typedef struct {
    User *users;
    int count;
    int max_count;
} Users;



Error read_file_users(FILE *fp, Users* users);

void print_user(User user);

Error add_user(Users *users, User user);

bool is_valid_user(User user);

bool is_unique_user(User user, Users users);

Error init_users(Users* users);

Error save_file_users(FILE *fp, Users users);

Error free_users(Users* users);

bool login(Users users, User* user);

bool set_tocken(char* login, int token, Users users);


typedef struct {
    int day;
    int month;
    int year;
    int hour;
    int minute;
    int second;
} Date;

Error init_user_interfase(User* user, Users users);

#endif