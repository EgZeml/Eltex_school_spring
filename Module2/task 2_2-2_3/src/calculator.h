#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdarg.h>

#define END_MARKER -9999.0
#define NUMBERS_LEN 6

// Объявление функций с переменным числом аргументов
double sum(double first, ...);
double sub(double first, ...);
double mult(double first, ...);
double div_op(double first, ...);

// Функция для выбора операции по номеру
double (*select_op(int choice)) (double, ...);

#endif // CALCULATOR_H
