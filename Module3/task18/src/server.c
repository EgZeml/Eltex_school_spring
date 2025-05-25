#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h>

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

/*
Запуск сервера: ./server <port>
Запуск клиента: ./client localhost <port>
*/

int main(int argc, char* argv[]) {
    char buff[1024]; // Буфер для различных нужд
    int listen_fd, client_fd; // дескрипторы сокетов
    int portno; // номер порта
    int pid; // id номер потока
    fd_set read_fds, master_fds; // структуры для select
    socklen_t clilen; // размер адреса клиента типа socklen_t
    struct sockaddr_in serv_addr, cli_addr; // структура сокета сервера и клиента

    printf("TCP SERVER DEMO\n");

    // ошибка в случае если мы не указали порт
    if (argc < 2) fatal("Использование: ./server порт");

    portno = atoi(argv[1]);

    // Перехват SIGCHLD
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    // 1) Создание сокета
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) fatal("Opening socket");
    
    // 2) Cвязывание сокета с адресом
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(listen_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) fatal("ERROR on binding");

    // 3) Jжидание подключений, размер очереди - 5
    listen(listen_fd, BACKLOG);
    clilen = sizeof(cli_addr);

    FD_ZERO(&master_fds); // Очищаем набор
    FD_ZERO(&read_fds);

    FD_SET(listen_fd, &master_fds);
    int fd_max = listen_fd;

    // Шаг 4 - извлекаем сообщение из очереди (цикл извлечения запросов на подключение)
    while (1) {
        // Принимаем сообщения клиента
        read_fds = master_fds;
        select(fd_max + 1, &read_fds, NULL, NULL, NULL);

        for (int i = 0; i <= fd_max; i++) {
            if (FD_ISSET(i, &read_fds)) {
                // Просьба о подключении
                if (i == listen_fd) {
                    int newsockfd = accept(listen_fd, (struct sockaddr*) &cli_addr, &clilen);
                    if (newsockfd < 0) fatal("ERROR on accept");
                    nclients++;

                    // Вывод сведений о клиенте
                    char str[INET_ADDRSTRLEN];
                    struct hostent* hst;
                    hst = gethostbyaddr((char*) &cli_addr.sin_addr, sizeof(cli_addr.sin_addr), AF_INET);
                    printf("+%s [%s] new connect!\n", hst ? hst->h_name : "Unknown host", inet_ntop(AF_INET, &cli_addr.sin_addr, str, sizeof(str)));
                    printusers(nclients);

                    // Отправка приглашения клиенту
                    recv_prompt(newsockfd);

                    // Добавление сокета в отслеживаемые
                    FD_SET(newsockfd, &master_fds);
                    if(newsockfd > fd_max) fd_max = newsockfd;
                // Данные на сокет от клиента
                } else {
                    // Обработка данных от клиента
                    int status = dostuff(i);
                    // Клиент закрыл соединение или отправил "quit"
                    if (status < 0) {
                        FD_CLR(i, &master_fds);
                        close(i);
                        nclients--;
                        printf("-disconnect\n");
                        printusers(nclients);
                    }
                }
            }      
        }
    }

    close(listen_fd);
    return 0;
}