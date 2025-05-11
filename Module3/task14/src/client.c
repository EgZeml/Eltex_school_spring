#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include "utils.h"

#define C_GREEN  "\x1b[32m"
#define C_CYAN   "\x1b[36m"
#define C_RESET  "\x1b[0m"

static void print_prompt(void) {
    printf(C_GREEN "Вы> " C_RESET);
    fflush(stdout);
}

static void child_receive_loop(int sock) {
    char buffer[BUFFER_SIZE];

    while (1) {
        ssize_t len = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
        if (len <= 0) {
            continue;
        }
        buffer[len] = '\0';

        if (strcmp(buffer, EXIT_MASSAGE) == 0) {
            kill(getppid(), SIGTERM);
            exit(EXIT_SUCCESS);
        }

        printf("\r\033[K" C_CYAN "Собеседник: %s\n" C_RESET, buffer);
        print_prompt();
    }
}

static void parent_send_loop(int sock, const struct sockaddr_in *srv_addr) {
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(*srv_addr);

    while (fgets(buffer, sizeof(buffer), stdin)) {
        size_t len = strcspn(buffer, "\n");
        buffer[len] = '\0';

        if (sendto(sock, buffer, len + 1, 0, (const struct sockaddr *)srv_addr, addr_len) < 0) {
            perror("sendto");
            break;
        }

        if (strcmp(buffer, EXIT_MASSAGE) == 0) {
            break;
        }

        print_prompt();
    }
}

int main(void) {
    int client_sock;
    struct sockaddr_in server_addr = {0};
    struct sockaddr_in client_addr = {0};
    char buffer[BUFFER_SIZE];
    socklen_t addr_len;
    pid_t pid;

    // 1) Создаем клиентский UDP-сокет и привязываем к системе
    client_sock = create_socket();
    create_client_data(&client_addr);
    if (bind(client_sock, (const struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        fatal("bind");
    }

    // 2) Заполняем данные сервера и отправляем CONNECT_MASSAGE
    create_server_data(&server_addr);
    sendto(client_sock, CONNECT_MASSAGE, strlen(CONNECT_MASSAGE) + 1, 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    // 3) Получаем идентификатор от сервера
    addr_len = sizeof(server_addr);
    ssize_t n = recvfrom(client_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, &addr_len);
    if (n < 0) {
        fatal("recvfrom");
    }
    buffer[n] = '\0';
    printf(C_CYAN "Сервер: %s\n" C_RESET, buffer);

    printf("Для использования: <id собеседника> текст\n");
    printf("Пример: 0 Тестовое сообщение\n");
    printf("Выход:  /exit\n");
    // 4) Форкаем процесс: потомок для приёма, родитель для отправки
    pid = fork();
    if (pid < 0) {
        close(client_sock);
        fatal("fork");
    }

    if (pid == 0) {
        child_receive_loop(client_sock);
    } else {
        // Разрешаем завершение по SIGTERM от потомка
        signal(SIGTERM, SIG_DFL);
        print_prompt();
        parent_send_loop(client_sock, &server_addr);
    }

    // 5) Завершаем
    close(client_sock);
    return EXIT_SUCCESS;
}