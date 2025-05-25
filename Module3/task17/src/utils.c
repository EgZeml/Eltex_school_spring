#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"

int create_socket() {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return socket_fd;
}

void create_client_data(struct sockaddr_in *client_addr) {
    memset(client_addr, 0, sizeof(*client_addr));
    client_addr->sin_family = AF_INET;
    client_addr->sin_addr.s_addr = INADDR_ANY;
    client_addr->sin_port = htons(0);  // порт 0 = ОС сама выдаст свободный
}

void create_server_data(struct sockaddr_in *server_addr) {
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    if (inet_aton("127.0.0.1", &server_addr->sin_addr) == 0) {
        perror("inet_aton");
        exit(1);
    }
    server_addr->sin_port = htons(51000);
}

// Завершение работы при ошибке
void fatal(const char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

int add_client(Client *clients, int *count, int max, const struct sockaddr_in *addr, int next_id) {
    if (*count >= max) {
        fprintf(stderr, "Server: too many clients (max=%d)\n", max);
        return -1;
    }
    clients[*count].id = next_id;
    clients[*count].addr = *addr;
    return (*count)++;
}

int find_client(const Client *clients, int count, int id) {
    for (int i = 0; i < count; i++) {
        if (clients[i].id == id) return i;
    }
    return -1;
}