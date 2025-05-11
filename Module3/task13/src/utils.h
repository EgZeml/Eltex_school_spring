#ifndef UTILS_H
#define UTILS_H

#include <semaphore.h>

#define BUFFER_CAPACITY 256     // Размер в байтах общей памяти
#define MAX_ELEMENTS 1000       // Максимальный размер набора сгенерированных чисел 
#define MAX_NUMBER 10000        // Максимальное сгенерированное число

// Имена семафоров
extern const char *SEM_GENERATE_SET;    // Следит за генерацией набора чисел
extern const char *SEM_MIN_MAX;         // Следит за обработкой набора числе
extern const char *SEM_RESULTS;         // Следит за выводом результатов работы

// Обработчик сигналов
void handler(int signo);

// Печатает ошибку и завершает программу
void fatal(const char *msg);

// Открывает или создаёт семафор
sem_t *open_semaphore(const char *name, int value);

// Закрывает и удаляет семафор
void sem_del(sem_t *sem, const char *name);

#endif