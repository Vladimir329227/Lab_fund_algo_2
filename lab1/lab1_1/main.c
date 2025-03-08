#include "main.h"
#include <stdio.h>
#include <string.h>

int main() { 
    int main_int;
    User user;
    bool is_login = false;

    Users users;
    init_users(&users);
    FILE *fp = fopen("filename.txt", "r+");
    if (fp == NULL) {
        return ERROR_FILE;
    }
    read_file_users(fp, &users);
    fclose(fp);
    while (true)
    {
        printf("1)Login \n2)Registration \n3)Exit\n");
        scanf("%i", &main_int);
        if (main_int == 1){
            printf("Enter login: ");
            scanf("%s", user.login);
            printf("Enter pin: ");
            scanf("%i", &user.pin);
            if (login(users, &user))
                is_login = true;
            else
                printf("Incorrect login or pin\n");
        }
        else if (main_int == 2){
            printf("Enter login: ");
            scanf("%s", user.login);
            printf("Enter pin: ");
            scanf("%i", &user.pin);
            user.cur_token = 0;
            user.max_token = -1;
            if (is_unique_user(user, users) && is_valid_user(user)){
                add_user(&users, user);
                is_login = true;
            }
            else
                printf("Incorrect login or pin\n");
        }
        else
            break;

        if(is_login){
            init_user_interfase(&user, users);
            is_login = false;
        }
    }
    
    
    
    FILE *fp2 = fopen("filename.txt", "w");
    if (fp2 == NULL) {
        return ERROR_FILE;
    }
    save_file_users(fp2, users);
    fclose(fp2);
    free_users(&users);
    return OK;
}
