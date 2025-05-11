#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define CONNECT_MASSAGE "Сообщение для подключения"
#define EXIT_MASSAGE "/exit"

/**
 * Создает UDP-сокет AF_INET/SOCK_DGRAM.
 * При ошибке выводит perror и завершает программу.
 * @return файловый дескриптор сокета
 */
int create_socket(void);

/**
 * Инициализирует структуру sockaddr_in для клиента.
 * Использует INADDR_ANY и порт 0.
 * @param client_addr указатель на структуру для заполнения
 */
void create_client_data(struct sockaddr_in *client_addr);

/**
 * Инициализирует структуру sockaddr_in для сервера.
 * По умолчанию адрес 127.0.0.1 и порт 51000.
 * @param server_addr указатель на структуру для заполнения
 */
void create_server_data(struct sockaddr_in *server_addr);

/**
 * Печатает сообщение об ошибке с strerror(errno) и завершает программу.
 * @param msg текст ошибки
 */
void fatal(const char *msg);

#endif // UTILS_H
