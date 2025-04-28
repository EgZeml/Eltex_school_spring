#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

// Посдчет площади квадрата
double square(double x) {
    return x * x;
}

// Вывод элементов массива с началом и концом
void print_values(int start, int finish, double* values) {
    for (int i = start; i < finish; i++) {
        printf("%.2f ", values[i]);
    }
    printf("\n");
}

// Создание массива числе из строки
void string_to_number(char* str[], double* out, int count) {
    for (int i = 0; i < count; i++) {
        char *endptr;
        errno = 0;  // сбрасываем errno перед вызовом

        double val = strtod(str[i+1], &endptr);

        // Проверяем, было ли что-то преобразовано
        if (endptr == str[i+1]) {
            fprintf(stderr, "Аргумент %d: «%s» не является числом\n", i+1, str[i+1]);
            exit(EXIT_FAILURE);
        }

        out[i] = val;
    }
}