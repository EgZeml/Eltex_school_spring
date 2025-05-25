#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include "utils.h"
#include "raw_utils.h"

#define C_GREEN  "\x1b[32m"
#define C_CYAN   "\x1b[36m"
#define C_RESET  "\x1b[0m"

static void print_prompt(void) {
    printf(C_GREEN "Вы> " C_RESET);
    fflush(stdout);
}

// Обработка принятых сообщений
static void child_receive_loop(int sock,
                               const char *server_ip, unsigned short server_port,
                               const char *client_ip, unsigned short client_port) {

    unsigned short sport = server_port;
    unsigned short dport = client_port;
    unsigned char  buffer[BUFFER_SIZE];
    char           msg[BUFFER_SIZE];
    struct iphdr  *ip;
    struct udphdr *udp;

    while (1) {
        // Принимаем полный IP-пакет
        ssize_t len = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
        if (len <= 0) {
            continue;
        }

        // Разбираем IP/UDP и копируем полезную нагрузку в msg
        int data_len = extract_ip_udp_data((char *)buffer, &ip, &udp, msg, len);
        if (data_len < 0) {
            continue;
        }

        // Фильтруем пакет по адресам и портам
        if (!packet_is_for_me(buffer, len,
                              inet_addr(server_ip), sport,
                              inet_addr(client_ip), dport)) {
            continue;
        }

        msg[data_len] = '\0';

        // Обрабатываем команду выхода
        if (strcmp(msg, EXIT_MESSAGE) == 0) {
            kill(getppid(), SIGTERM);
            exit(EXIT_SUCCESS);
        }

        // Выводим сообщение от "собеседника"
        printf("\r\033[K" C_CYAN "Собеседник: %s\n" C_RESET, msg);
        print_prompt();
    }
}

// Отправка сообщений
static void parent_send_loop(int sock, const struct sockaddr_in *srv_addr, 
                             const char *src_ip, unsigned short src_port, 
                             const char *dst_ip, unsigned short dst_port) {

    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(*srv_addr);

    while (fgets(buffer, sizeof(buffer), stdin)) {
        ssize_t len = strcspn(buffer, "\n");
        buffer[len] = '\0';

        // Отправка сообщения
        if (raw_sendto(sock, buffer, len + 1, src_ip, src_port, dst_ip, dst_port, srv_addr) < 0) {
            perror("raw_sendto");
            break;
        }

        // Обработка выхода
        if (strcmp(buffer, EXIT_MESSAGE) == 0) {
            break;
        }

        print_prompt();
    }
}

int main(void) {
    int client_sock;
    struct sockaddr_in server_addr = {0}, client_addr = {0};
    char server_ip[INET_ADDRSTRLEN], client_ip[INET_ADDRSTRLEN];
    unsigned short server_port, client_port;

    // 1) Создаём raw-сокет
    client_sock = create_raw_socket();

    // 2) Создаем временный вспомогательный UDP сокет
    int tmp = socket(AF_INET, SOCK_DGRAM, 0);
    client_port = get_ephemeral_port(tmp);

    // 3) Подготовим bind() с тем же IP и портом
    client_addr.sin_family      = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port        = htons(client_port);
    if (bind(client_sock,
             (struct sockaddr *)&client_addr,
             sizeof(client_addr)) < 0)
    {
        fatal("bind");                                              
    }

    // 4) Заполняем адрес сервера и отправляем CONNECT_MASSAGE
    create_server_data(&server_addr);    

    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    client_port = ntohs(client_addr.sin_port);

    inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, sizeof(server_ip));
    server_port = ntohs(server_addr.sin_port);

    if (raw_sendto(client_sock, CONNECT_MESSAGE, strlen(CONNECT_MESSAGE) + 1,
                   client_ip, client_port,
                   server_ip, server_port,
                   &server_addr) < 0) {
        perror("raw_sendto");
        exit(EXIT_FAILURE);
    }

    // 5) Получаем ID от сервера
    unsigned char pktbuf[BUFFER_SIZE];
    socklen_t al = sizeof(server_addr);
    ssize_t n;

    // Ждём именно пакет от сервера
    do {
        n = recvfrom(client_sock, pktbuf, sizeof(pktbuf), 0,
                    (struct sockaddr *)&server_addr, &al);
        if (n < 0) fatal("recvfrom");
    } while (!packet_is_for_me(pktbuf, n, inet_addr(server_ip), server_port, inet_addr(client_ip), client_port));

    // Разбираем IP/UDP и получаем payload
    struct iphdr  *ip;
    struct udphdr *udp;
    char msg[BUFFER_SIZE];
    int len = extract_ip_udp_data(pktbuf, &ip, &udp, msg, n);
    if (len < 0) {
        fprintf(stderr, "Ошибка разбора пакета\n");
        close(client_sock);
        return EXIT_FAILURE;
    }

    // Завершаем строку и выводим сообщение
    msg[len] = '\0';
    printf(C_CYAN "Сервер: %s\n" C_RESET, msg);

    printf("Для использования: <id собеседника> текст\n");
    printf("Пример: 0 Тестовое сообщение\n");
    printf("Выход:  /exit\n");

    // 6) Форкаем: потомок читает, родитель пишет
    pid_t pid = fork();
    if (pid < 0) {
        close(client_sock);
        fatal("fork");
    }
    if (pid == 0) {
        child_receive_loop(client_sock, server_ip, server_port, client_ip, client_port);
    } else {
        signal(SIGTERM, SIG_DFL);
        print_prompt();
        parent_send_loop(client_sock,
                         &server_addr,
                         client_ip, client_port,
                         server_ip, server_port);
    }

    close(tmp);
    close(client_sock);
    return EXIT_SUCCESS;
}