#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "utils.h"

#define BACKLOG 5

static volatile int nclients = 0;

// Обработка завершения дочерних процессов
void sigchld_handler(int signo) {
    // Перехватываем всех завершившихся детей
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        nclients--;
        printusers(nclients);
    }
}

int main(int argc, char* argv[]) {
    char buff[1024]; // Буфер для различных нужд
    int sockfd, newsockfd; // дескрипторы сокетов
    int portno; // номер порта
    int pid; // id номер потока
    socklen_t clilen; // размер адреса клиента типа socklen_t
    struct sockaddr_in serv_addr, cli_addr; // структура сокета сервера и клиента

    printf("TCP SERVER DEMO\n");

    // ошибка в случае если мы не указали порт
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(EXIT_FAILURE);
    }

    portno = atoi(argv[1]);

    // Перехват SIGCHLD
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    // 1) Создание сокета
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) fatal("Opening socket");
    
    // Шаг 2 - связывание сокета с адресом
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) fatal("ERROR on binding");

    // Шаг 3 - ожидание подключений, размер очереди - 5
    listen(sockfd, BACKLOG);
    clilen = sizeof(cli_addr);

    // Шаг 4 - извлекаем сообщение из очереди (цикл извлечения запросов на подключение)
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);
        if (newsockfd < 0) fatal("ERROR on accept");
        nclients++;

        // Вывод сведений о клиенте
        char str[INET_ADDRSTRLEN];
        struct hostent* hst;
        hst = gethostbyaddr((char*) &cli_addr.sin_addr, sizeof(cli_addr.sin_addr), AF_INET);
        printf("+%s [%s] new connect!\n", hst ? hst->h_name : "Unknown host", inet_ntop(AF_INET, &cli_addr.sin_addr, str, sizeof(str)));
        printusers(nclients);

        pid = fork();
        if (pid < 0) fatal("ERROR on fork");
        if (pid == 0) {
            dostuff(newsockfd);
            printf("-disconnect\n");
            exit(0);
        } else close(newsockfd);
    }
    close(sockfd);
    return 0;
}