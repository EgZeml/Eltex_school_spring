#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "utils.h"

int main(int argc, char* argv[]) {
    int my_sock, portno, n;
    struct sockaddr_in serv_addr = { 0 };
    struct hostent* server;
    char buf[1024];

    printf("TCP DEMO CLIENT\n");

    if (argc < 3) fatal("Использование: ./client hostname port\n");

    portno = atoi(argv[2]);

    // 1) Создание сокета
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) fatal("Opening socket");

    // Извлечение хоста
    server = gethostbyname(argv[1]);
    if (server == NULL) fatal("No such host");

    // Заполнение структуры serv_addr
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(portno);

    // 2) Установка соединения
    if (connect(my_sock, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) fatal("Connect");

    // Считывание приветственного сообщения
    n = recv(my_sock, buf, sizeof(buf) - 1, 0);
    if (n < 0) {
        fatal("Recv error");
    }
    if (n == 0) {
        // сервер закрыл соединение
        fatal("Recv error");;
    }
    buf[n] = '\0';
    printf("S=>C: %s", buf);

    // 3) Чтение и передача сообщений
    while(1) {        
        // Запрашиваем ввод пользователя
        printf("C=>S: ");
        if (!fgets(buf, sizeof(buf), stdin)) break;

        // Отправка файла
        if (strncmp(buf, "sendfile ", 9) == 0) {
            // вырезаем путь
            char *path = buf + 9;
            path[strcspn(path, "\n")] = '\0';
            if (send_file(my_sock, path) < 0) {
                fprintf(stderr, "Ошибка отправки файла\n");
            }
            continue;
        }        

        // Отправляем введённое
        if (send(my_sock, buf, strlen(buf), 0) < 0) {
            fprintf(stderr, "Ошибка отправки файла\n");
            continue;
        }

        // Если это quit — ожидаем прощальное сообщение и выходим
        if (strncmp(buf, "quit", 4) == 0) {
            n = recv(my_sock, buf, sizeof(buf) - 1, 0);
            if (n > 0) {
                buf[n] = '\0';
                printf("S=>C: %s", buf);  // здесь печатаем "Bye!\n"
            }
            printf("Exit...\n");
            close(my_sock);
            return EXIT_SUCCESS;
        }
    
        // Принимаем результат
        n = recv(my_sock, buf, sizeof(buf) - 1, 0);
        if (n < 0) {
            fatal("Recv error");
        }
        if (n == 0) {
            // сервер закрыл соединение
            break;
        }
        buf[n] = '\0';
        printf("S=>C: %s", buf);
    }
    close(my_sock);
    return EXIT_FAILURE;
}