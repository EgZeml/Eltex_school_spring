#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>   // Для exit, EXIT_FAILURE
#include <stdio.h>    // Для perror, printf, snprintf
#include <unistd.h>   // Для read, write

// Функция обработки ошибок
void fatal(const char *msg);

// Основная рабочая функция, взаимодействующая по сокету
void dostuff(int socket);

// Печать количества активных пользователей
void printusers(int nclients);

// Разбор сообщения клиента
int parse_input(char *str, char *tokens[], int max_tokens);

#endif // UTILS_H
