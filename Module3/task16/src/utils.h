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

// Отправить файл по сокету: сначала заголовок, потом содержимое
int send_file(int sock, char* path);

// Принять файл на сервере и записать в текущую директорию
int recv_file(int sock);

#endif // UTILS_H
