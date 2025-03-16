#define _POSIX_C_SOURCE 199309L 
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define TIME_TO_deadlock 1000

sem_t *forks;
sem_t limit;
int num_philos;
volatile int running = 1;
int use_solution;

void delay_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

void *philosopher(void *arg) {
    int id = *(int *)arg;
    free(arg);
    int left = id;
    int right = (id + 1) % num_philos;

    while (running) {
        printf("Philosopher %d is thinking\n", id);
        delay_ms(rand() % 500 + 500);

        printf("Philosopher %d is hungry\n", id);

        if (use_solution) {
            sem_wait(&limit);
        }
        sem_wait(&forks[left]);
        delay_ms(TIME_TO_deadlock);
        sem_wait(&forks[right]);

        printf("Philosopher %d is eating\n", id);
        delay_ms(rand() % 500 + 500);

        sem_post(&forks[right]);
        sem_post(&forks[left]);
        if (use_solution) {
            sem_post(&limit);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <num_philos> <time_sec> <use_solution (0|1)>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *endptr;
    num_philos = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || num_philos <= 0) {
        fprintf(stderr, "Invalid number of philosophers\n");
        return EXIT_FAILURE;
    }

    int time_sec = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || time_sec <= 0) {
        fprintf(stderr, "Invalid time\n");
        return EXIT_FAILURE;
    }

    use_solution = strtol(argv[3], &endptr, 10);
    if (*endptr != '\0' || (use_solution != 0 && use_solution != 1)) {
        fprintf(stderr, "Invalid use_solution flag (must be 0 or 1)\n");
        return EXIT_FAILURE;
    }

    forks = malloc(num_philos * sizeof(sem_t));
    if (!forks) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < num_philos; i++) {
        if (sem_init(&forks[i], 0, 1) != 0) {
            perror("sem_init forks");
            for (int j = 0; j < i; j++) {
                sem_destroy(&forks[j]);
            }
            free(forks);
            return EXIT_FAILURE;
        }
    }

    if (use_solution) {
        if (sem_init(&limit, 0, num_philos - 1) != 0) {
            perror("sem_init limit");
            for (int i = 0; i < num_philos; i++) {
                sem_destroy(&forks[i]);
            }
            free(forks);
            return EXIT_FAILURE;
        }
    }

    pthread_t *threads = malloc(num_philos * sizeof(pthread_t));
    if (!threads) {
        perror("malloc threads");
        if (use_solution) sem_destroy(&limit);
        for (int i = 0; i < num_philos; i++) {
            sem_destroy(&forks[i]);
        }
        free(forks);
        return EXIT_FAILURE;
    }

    srand(time(NULL));
    for (int i = 0; i < num_philos; i++) {
        int *id = malloc(sizeof(int));
        if (!id) {
            perror("malloc id");
            continue;
        }
        *id = i;
        if (pthread_create(&threads[i], NULL, philosopher, id) != 0) {
            perror("pthread_create");
            free(id);
        }
    }

    sleep(time_sec);
    running = 0;

    for (int i = 0; i < num_philos; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < num_philos; i++) {
        sem_destroy(&forks[i]);
    }
    free(forks);

    if (use_solution) {
        sem_destroy(&limit);
    }

    free(threads);

    return EXIT_SUCCESS;
}