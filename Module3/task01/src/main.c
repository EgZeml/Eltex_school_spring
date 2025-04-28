#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"

int main(int argc, char* argv[]) {
    // Не переданы стороны квадрата
    if (argc < 2) {
        printf("Пример запуска программы: ./main arg1 arg2 ...\n");
        exit(EXIT_FAILURE);
    }

    int num_args = argc - 1;
    double values[num_args];

    // Преобразование переданных аргументов (стороны квадрата) в число
    string_to_number(argv, values, num_args);

    int mid = num_args / 2;
    pid_t pid;

    // Создание дочернего процесса
    pid = fork();

    switch (pid) {
    // Ошибка создании процесса
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
    // Дочерний процесс
    // Считает первую половину, затем выводит результаты и отправляет сигнал о завершении родительскому процессу
    case 0:
        for (int i = 0; i < mid; i++) {
            values[i] = square(values[i]);
        }
        
        printf("Первая половина, подсчитанная дочерним процессом: ");
        print_values(0, mid, values);

        exit(EXIT_SUCCESS);
    // Родительский процесс
    // Считает вторую половину, затем ждет сигнал о завершении дочернего процесса и выводит результаты
    default:
        for (int i = mid; i < num_args; i++) {
            values[i] = square(values[i]);
        }

        int status;
        wait(&status);
        printf("Вторая половина, подсчитанная родительским процессом: ");
        print_values(mid, num_args, values);
    }

    return EXIT_SUCCESS;
}