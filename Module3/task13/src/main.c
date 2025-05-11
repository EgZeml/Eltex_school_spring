#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include "utils.h"

// Флаг работы процесса
volatile sig_atomic_t working = 1;

int main() {

    int processed_sets = 0;

    sem_t *sem_generate_set = open_semaphore(SEM_GENERATE_SET, 1);
    if (sem_generate_set == SEM_FAILED) exit(EXIT_FAILURE);

    sem_t *sem_min_max = open_semaphore(SEM_MIN_MAX, 0);
    if (sem_min_max == SEM_FAILED) {
        sem_del(sem_generate_set, SEM_GENERATE_SET);
        exit(EXIT_FAILURE);
    }

    sem_t *sem_results = open_semaphore(SEM_RESULTS, 0);
    if (sem_results == SEM_FAILED) {
        sem_del(sem_generate_set, SEM_GENERATE_SET);
        sem_del(sem_min_max, SEM_MIN_MAX);
        exit(EXIT_FAILURE);
    }

    int fd = shm_open("/my_shm", O_CREAT | O_RDWR, 0666);
    if (fd == -1) fatal("shm_open");
    if (ftruncate(fd, BUFFER_CAPACITY) == -1) fatal("shm_open");
    int *shm_addr = mmap(NULL, BUFFER_CAPACITY, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm_addr == MAP_FAILED) fatal("mmap");

    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    // Убираем параметр рестарта ожидания после сигнала завершения
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) fatal("sigaction");

    // Форкаем процесс
    pid_t pid = fork();
    if (pid < 0) {
        fatal("fork");
    } else if (pid == 0) {
        // Дочерний процесс
        while (working) {
            // 2) Обрабатываем числа в общей памяти
            int max_num = INT_MIN, min_num = INT_MAX;
            while (1) {
                // 2.1) Захватывает ресурс, начинаю обработку чисел из общей памяти
                if (sem_wait(sem_min_max) == -1) {
                    if (errno == EINTR) break;
                    fatal("sem_wait");
                }
                int chunk = shm_addr[0];
                // Набор окончен
                if (chunk == 0) break;
                for (int j = 0; j < chunk; j++) {
                    int v = shm_addr[j + 1];
                    if (v > max_num) max_num = v;
                    if (v < min_num) min_num = v;
                }
                // 2.2) Освобождаем ресурс, позволяя родительскому процессу записать следующую часть чисел из набора
                sem_post(sem_generate_set);
            }
            shm_addr[0] = max_num;
            shm_addr[1] = min_num;
            // 2.3) Освобождаем ресурс, чтобы родительский процесс начал работу с результами работы дочернего процесса
            sem_post(sem_results);
        }
        close(fd);
        _exit(EXIT_SUCCESS);
    } else {
        // Родительский процесс
        srand(time(NULL) ^ getpid());
        // 1) Начинаем генерировать и записывать числа в общую память
        while (working) {
            int total = 1 + rand() % MAX_ELEMENTS;
            int remain = total;
            // 1.1) Отправляем набор в общую память по частям
            while (remain > 0) {
                int chunk = remain < (BUFFER_CAPACITY/sizeof(int) - 1) ? remain : (BUFFER_CAPACITY/sizeof(int) - 1);
                remain -= chunk;
                // 1.2) Захватывает ресурс, генерируя и записывая числа в общую память
                if (sem_wait(sem_generate_set) == -1) {
                    if (errno == EINTR) break;
                    fatal("sem_wait");
                }
                shm_addr[0] = chunk;
                for (int j = 0; j < chunk; j++)
                    shm_addr[j + 1] = rand() % MAX_NUMBER;
                // 1.3) Освобождаем ресурс, чтобы дочерний процесс начал обработку чисел в общей памяти 
                sem_post(sem_min_max);
            }
            // 1.4) В конце отсылаем дочернему процессу 0, говоря о том, что набор закончился
            sem_wait(sem_generate_set);
            shm_addr[0] = 0;
            sem_post(sem_min_max);

            // 3) Работа с результами дочернего процесса
            // 3.1) Захватываем ресурс для вывода разультатов 
            if (sem_wait(sem_results) == -1) {
                if (errno == EINTR) break;
                fatal("sem_wait");
            }
            processed_sets++;
            printf("Максимальное число: %d \nМинимальное число: %d\n", shm_addr[0], shm_addr[1]);
            // Спим для более читаемого вывода
            sleep(1);
            // 3.2) Освобождаем ресурс для генерации следующего набора чисел
            sem_post(sem_generate_set);
        }
        // Дополнительный сигнал дочернему процессу, если они не в одной группе
        kill(pid, SIGINT);
        waitpid(pid, NULL, 0);
    }

    printf("\nОбработанно наборов: %d\n\n", processed_sets);

    // Очистка и удаление
    munmap(shm_addr, 1024);
    close(fd);
    shm_unlink("/my_shm");
    sem_del(sem_generate_set, SEM_GENERATE_SET);
    sem_del(sem_min_max, SEM_MIN_MAX);
    sem_del(sem_results, SEM_RESULTS);
    return 0;
}