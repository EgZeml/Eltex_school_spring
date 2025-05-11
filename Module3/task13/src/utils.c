#define _POSIX_C_SOURCE 200809L

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

// Имена семафоров
const char *SEM_GENERATE_SET = "/sem_generate_set";
const char *SEM_MIN_MAX      = "/sem_min_max";
const char *SEM_RESULTS      = "/sem_results";

// Обработчик сигнала завершения
void handler(int signo) {
    // Помечаем, что переменная в другом файле
    extern volatile sig_atomic_t working;
    working = 0;
}

// Завершение работы при ошибке
void fatal(const char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

// Открытие семафора
sem_t *open_semaphore(const char *name, int value) {
    sem_t *sem = sem_open(name, O_CREAT | O_EXCL, 0666, value);
    if (sem == SEM_FAILED && errno == EEXIST) {
        sem = sem_open(name, 0);
    }
    if (sem == SEM_FAILED) {
        fprintf(stderr, "sem_open(%s): %s\n", name, strerror(errno));
    }
    return sem;
}

// Удаление семафора
void sem_del(sem_t *sem, const char *name) {
    sem_close(sem);
    sem_unlink(name);
}