#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define CONNECT_MESSAGE "Сообщение для подключения"
#define EXIT_MESSAGE "/exit"

/**
 * @brief Структура для хранения данных о клиенте.
 */
typedef struct {
    int id;                      /**< Уникальный идентификатор клиента */
    struct sockaddr_in addr;     /**< Адрес клиента */
} Client;

/**
 * @brief Создание UDP-сокета AF_INET/SOCK_DGRAM.
 * @return Дескриптор сокета, завершает работу при ошибке.
 */
int create_socket(void);

/**
 * @brief Инициализация структуры sockaddr_in для клиента (INADDR_ANY, порт 0).
 * @param client_addr Указатель на структуру sockaddr_in.
 */
void create_client_data(struct sockaddr_in *client_addr);

/**
 * @brief Инициализация структуры sockaddr_in для сервера (127.0.0.1:51000).
 * @param server_addr Указатель на структуру sockaddr_in.
 */
void create_server_data(struct sockaddr_in *server_addr);

/**
 * @brief Вывод сообщения об ошибке и завершение программы.
 * @param msg Сообщение об ошибке.
 */
void fatal(const char *msg);

/**
 * @brief Добавить клиента в массив.
 * @param clients  Массив клиентов.
 * @param count    Указатель на текущее количество клиентов.
 * @param max      Максимальное количество клиентов.
 * @param addr     Адрес клиента.
 * @param next_id  Следующий свободный идентификатор.
 * @return Идентификатор нового клиента или -1 при ошибке.
 */
int add_client(Client *clients, int *count, int max, const struct sockaddr_in *addr, int next_id);

/**
 * @brief Найти клиента по id.
 * @param clients Массив клиентов.
 * @param count   Количество клиентов.
 * @param id      Идентификатор для поиска.
 * @return Индекс клиента или -1, если не найден.
 */
int find_client(const Client *clients, int count, int id);

#endif // UTILS_H
