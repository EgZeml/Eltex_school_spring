#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"
#include "raw_utils.h"
#define MAX_CLIENTS 2

int main(void) {
    int server_sock;
    struct sockaddr_in server_addr = {0}, remote_addr = {0};
    socklen_t addr_len = sizeof(remote_addr);
    Client clients[MAX_CLIENTS];
    int next_id = 0, client_count = 0;
    char buffer[BUFFER_SIZE];

    // 1) Создаем UDP-сокет
    server_sock = create_socket();
    if (server_sock < 0) fatal("create_socket");

    // 2) Настраиваем адрес сервера и привязываем сокет
    create_server_data(&server_addr);
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fatal("bind");
    }

    // 3) Основной цикл приёма сообщений
    while (1) {
        ssize_t n = recvfrom(server_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&remote_addr, &addr_len);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }
        buffer[n] = '\0';

        // Новое подключение
        if (strcmp(buffer, CONNECT_MESSAGE) == 0) {
            if (add_client(clients, &client_count, MAX_CLIENTS, &remote_addr, next_id) < 0)
                continue;

            char msg[BUFFER_SIZE];
            int len = snprintf(msg, sizeof(msg), "Ваш идентификатор: %d", next_id);
            if (len < 0 || len >= (int)sizeof(msg)) {
                fprintf(stderr, "Server: snprintf error\n");
                continue;
            }
            if (sendto(server_sock, msg, len + 1, 0, (struct sockaddr *)&remote_addr, addr_len) < 0) {
                perror("sendto");
            }
            next_id++;
        }
        // Запрос на выход
        else if (strcmp(buffer, EXIT_MESSAGE) == 0) {
            for (int i = 0; i < client_count; i++) {
                if (sendto(server_sock, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&clients[i].addr, addr_len) < 0) {
                    perror("sendto");
                }
            }
            break;
        }
        // Обычное сообщение: формат "<id> <текст>"
        else {
            char *p;
            long id = strtol(buffer, &p, 10);
            if (p == buffer || *p != ' ') {
                fprintf(stderr, "Server: invalid message format '%s'\n", buffer);
                continue;
            }
            const char *text = p + 1;
            int idx = find_client(clients, client_count, (int)id);
            if (idx < 0) {
                fprintf(stderr, "Server: client id %ld not found\n", id);
                continue;
            }
            if (sendto(server_sock, text, strlen(text) + 1, 0, (const struct sockaddr *)&clients[idx].addr, addr_len) < 0) {
                perror("sendto");
            }
        }
    }

    close(server_sock);
    return EXIT_SUCCESS;
}
