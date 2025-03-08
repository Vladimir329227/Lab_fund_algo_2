#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "main.h"


bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int days_in_month(int month, int year) {
    if (month == 2)
        return is_leap_year(year) ? 29 : 28;
    else if (month == 4 || month == 6 || month == 9 || month == 11)
        return 30;
    else 
        return 31;
}

bool is_valid_date_time(Date dt) {
    if (dt.year < 1)
        return false;

    if (dt.month < 1 || dt.month > 12)
        return false;

    if (dt.day < 1 || dt.day > days_in_month(dt.month, dt.year))
        return false;

    if (dt.hour < 0 || dt.hour > 23)
        return false;

    if (dt.minute < 0 || dt.minute > 59)
        return false;

    if (dt.second < 0 || dt.second > 59)
        return false;

    return true;
}

long long time_difference(Date dt1, Date dt2) {
    long long total_seconds1 = 0, total_seconds2 = 0;

    for (int y = 1; y < dt1.year; ++y) {
        total_seconds1 += is_leap_year(y) ? 366 * 24 * 60 * 60 : 365 * 24 * 60 * 60;
    }
    for (int m = 1; m < dt1.month; ++m) {
        total_seconds1 += days_in_month(m, dt1.year) * 24 * 60 * 60;
    }
    total_seconds1 += (dt1.day - 1) * 24 * 60 * 60;
    total_seconds1 += dt1.hour * 60 * 60;
    total_seconds1 += dt1.minute * 60;
    total_seconds1 += dt1.second;

    for (int y = 1; y < dt2.year; ++y) {
        total_seconds2 += is_leap_year(y) ? 366 * 24 * 60 * 60 : 365 * 24 * 60 * 60;
    }
    for (int m = 1; m < dt2.month; ++m) {
        total_seconds2 += days_in_month(m, dt2.year) * 24 * 60 * 60;
    }
    total_seconds2 += (dt2.day - 1) * 24 * 60 * 60;
    total_seconds2 += dt2.hour * 60 * 60;
    total_seconds2 += dt2.minute * 60;
    total_seconds2 += dt2.second;

    return total_seconds1 - total_seconds2;
}


Error init_user_interfase(User* user, Users users){
    char main_str[256];
    char flag;
    time_t now_time;
    struct tm* now_time_tm = {0};
    long long difference;
    Date date, date_now;
    
    int num = 0;
    char login_user[6], ch;

    while (true)
    {
        num = 0;
        if (user->cur_token == user->max_token)
            break;
        printf("Enter command:");
        while ((ch = getchar()) != EOF && ch != '\n')
            main_str[num++] = ch;
    
        main_str[num++] = '\0';
        
        if (strcmp(main_str, "Time") == 0){
            user->cur_token++;
            now_time = time(NULL);
            now_time_tm = localtime(&now_time);
            printf("Cur time: %02d:%02d:%02d\n",
                now_time_tm->tm_hour, now_time_tm->tm_min, now_time_tm->tm_sec);
        }
        else if (strcmp(main_str, "Date") == 0)
        {
            user->cur_token++;
            now_time = time(NULL);
            now_time_tm = localtime(&now_time);
            printf("Cur date: %02d:%02d:%04d\n",
                now_time_tm->tm_mday, now_time_tm->tm_mon + 1, now_time_tm->tm_year + 1900);
        }
        else if (sscanf(main_str, "Howmuch %i:%i:%i %i:%i:%i -%c", 
            &date.day, &date.month, &date.year,
            &date.hour, &date.minute, &date.second,
            &flag) == 7)
        {
            user->cur_token++;
            if (is_valid_date_time(date)){
                difference = time_difference(date_now, date);
                now_time = time(NULL);
                now_time_tm = localtime(&now_time);
                date_now.day = now_time_tm->tm_mday;
                date_now.month = now_time_tm->tm_mon + 1;
                date_now.year = now_time_tm->tm_year + 1900;
                date_now.hour = now_time_tm->tm_hour;
                date_now.minute = now_time_tm->tm_min;
                date_now.second = now_time_tm->tm_sec;
                difference = time_difference(date_now, date);
                switch (flag)
                {
                case 's':
                    printf("Result: %lld\n", difference);
                    break;
                case 'm':
                    printf("Result: %lld\n", difference / 60);
                    break;
                case 'h':
                    printf("Result: %lld\n", difference / 60 / 60);
                    break;
                case 'y':
                    printf("Result: %d\n", date_now.year - date.year);
                    break;
                default:
                    break;
                }
            }
            else
                printf("Invalid data\n");
        }
        else if (strcmp(main_str, "Logout") == 0)
            break;
        else if (sscanf(main_str, "Sanctions %s %i", 
            login_user,&num) == 2)
        {
            user->cur_token++;
            printf("Confirm the command:");
            scanf("%s", main_str);
            if(strcmp(main_str, "12345") == 0){
                set_tocken(login_user, num, users);
                if (user->cur_token >= user->max_token && !(user->max_token < 0))
                    break;
            }
            else
                printf("Wrong password\n");
        }
        else
            printf("Invalid enter\n");  
    }
    
    return OK;
}