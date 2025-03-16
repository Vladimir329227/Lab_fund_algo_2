#define _POSIX_C_SOURCE 199309L 
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

typedef enum { 
    GENDER_NONE = -1,
    GENDER_WOMEN = 0, 
    GENDER_MAN = 1 
} gender_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int count[2];        
    int max_capacity;
    gender_t current_gender;
} bathroom_t;

void person_wants_to_enter(bathroom_t *bathroom, gender_t gender) {
    pthread_mutex_lock(&bathroom->lock);
    const gender_t other_gender = (gender == GENDER_WOMEN) ? GENDER_MAN : GENDER_WOMEN;
    
    while (bathroom->count[other_gender] > 0 || 
          (bathroom->count[0] + bathroom->count[1]) >= bathroom->max_capacity ||
          (bathroom->current_gender != GENDER_NONE && bathroom->current_gender != gender)) {
        pthread_cond_wait(&bathroom->cond, &bathroom->lock);
    }
    
    if (bathroom->current_gender == GENDER_NONE) {
        bathroom->current_gender = gender;
    }
    bathroom->count[gender]++;
    
    pthread_mutex_unlock(&bathroom->lock);
}

void person_leaves(bathroom_t *bathroom, gender_t gender) {
    pthread_mutex_lock(&bathroom->lock);
    
    bathroom->count[gender]--;
    if (bathroom->count[gender] == 0) {
        bathroom->current_gender = GENDER_NONE;
    }
    
    pthread_cond_broadcast(&bathroom->cond);
    pthread_mutex_unlock(&bathroom->lock);
}

void* person_thread(void* arg) {
    bathroom_t* bathroom = (bathroom_t*)arg;
    gender_t gender = rand() % 2;
    
    person_wants_to_enter(bathroom, gender);
    
    pthread_mutex_lock(&bathroom->lock);
    printf("%s entered. Women: %d, Men: %d\n", 
           gender == GENDER_WOMEN ? "Woman" : "Man",
           bathroom->count[GENDER_WOMEN], 
           bathroom->count[GENDER_MAN]);
    pthread_mutex_unlock(&bathroom->lock);
    
    sleep(1 + rand() % 2);
    
    person_leaves(bathroom, gender);
    
    pthread_mutex_lock(&bathroom->lock);
    printf("%s left. Women: %d, Men: %d\n", 
           gender == GENDER_WOMEN ? "Woman" : "Man",
           bathroom->count[GENDER_WOMEN], 
           bathroom->count[GENDER_MAN]);
    pthread_mutex_unlock(&bathroom->lock);
    
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return ERROR_INPUT;
    }
    
    char* endptr;
    long N = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || N <= 0) {
        fprintf(stderr, "Invalid input\n");
        return ERROR_INPUT;
    }
    
    bathroom_t bathroom = {
        .count = {0, 0},
        .max_capacity = (int)N,
        .current_gender = GENDER_NONE
    };
    
    if (pthread_mutex_init(&bathroom.lock, NULL) != 0 ||
        pthread_cond_init(&bathroom.cond, NULL) != 0) {
        perror("Init error");
        return 1;
    }
    
    const int THREADS = 10;
    pthread_t threads[THREADS];
    
    for (int i = 0; i < THREADS; i++)
        if (pthread_create(&threads[i], NULL, person_thread, &bathroom) != 0)
            return ERROR;
    
    for (int i = 0; i < THREADS; i++)
        if (pthread_join(threads[i], NULL) != 0)
            return ERROR;
    
    pthread_mutex_destroy(&bathroom.lock);
    pthread_cond_destroy(&bathroom.cond);
    
    return OK;
}