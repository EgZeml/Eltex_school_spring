#include <string.h>
#include <math.h>
#include <fcntl.h>    
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "utils.h"
#include "math.h"

#define MAX_TOKENS 3
#define EXIT_MSG "quit"

void fatal(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int parse_input(char *str, char *tokens[], int max_tokens) {
    int count = 0;
    char *p = str;
    while (*p && count < max_tokens) {
        // Пропустить пробелы и заменить их на 0
        while (*p == ' ') {
            *p = 0;
            p++;
        }
        if (*p == 0) break;
        // Указатель на начало токена
        tokens[count++] = p;
        // Перейти до следующего пробела или конца
        while (*p && *p != ' ') {
            p++;
        }
    }
    return count;
}

// Печать количества активных пользователей
void printusers(int nclients) {
    if(nclients) {
        printf("%d user on-line\n", nclients);
    } else {
        printf("No User on line\n");
    }
}

// Отправить файл по сокету: сначала заголовок, потом содержимое
int send_file(int sock, char* path) {
    struct stat st;
    if (stat(path, &st) < 0) return -1;
    off_t filesize = st.st_size;
    const char *filename = strrchr(path, '/');
    if (filename) 
        filename++;
    else
        filename = path;

    // Заголовок: FILE имя размер\n
    char header[256];
    int hlen = snprintf(header, sizeof(header), "FILE %s %ld\n", filename, (long)filesize);
    if (send(sock, header, hlen, 0) < 0) return -1;

    // Открываем файл и шлём его блоками
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    char buf[1024];
    ssize_t r;
    while ((r = read(fd, buf, sizeof(buf))) > 0) {
        if (send(sock, buf, r, 0) != r) {
            close(fd); 
            return -1; 
        }
    }
    close(fd);
    return (r < 0) ? -1 : 0;
}

// Принять файл на сервере и записать в текущую директорию
int recv_file(int sock) {
    // Сначала читаем заголовок целиком
    char header[256];
    int pos = 0;
    while (pos < (int)sizeof(header) - 1) {
        ssize_t n = recv(sock, &header[pos], 1, 0);
        if (n <= 0) return -1;
        if (header[pos] == '\n') break;
        pos++;
    }
    header[pos] = '\0';

    // Парсим: FILE имя размер
    char cmd[16], filename[128];
    long filesize;
    if (sscanf(header, "%15s %127s %ld", cmd, filename, &filesize) != 3) return -1;
    
    char new_filename[256];
    snprintf(new_filename, sizeof(new_filename), "server_%s", filename);

    int fd = open(new_filename, O_CREAT|O_WRONLY|O_TRUNC, 0666);
    if (fd < 0) return -1;

    // Приём содержимого
    long remaining = filesize;
    char buf[1024];
    while (remaining > 0) {
        ssize_t n = recv(sock, buf, sizeof(buf) < remaining ? sizeof(buf) : remaining, 0);
        if (n <= 0) { 
            close(fd); 
            return -1; 
        }
        write(fd, buf, n);
        remaining -= n;
    }
    close(fd);
    return 0;
}

void dostuff(int socket) {
    char buff[256];
    char *tokens[MAX_TOKENS];
    char *prompt =
        "Введите команду (например, \"sum 3 4\" или \"quit\")\n"
        "Доступные команды:\n"
        "  sum    - сумма\n"
        "  sub    - разность\n"
        "  mult   - умножение\n"
        "  div_op - деление\n"
        "  quit   - выход\n"
        "  sendfile <path> - Отправить файл\n";


    // Просим команду
    if (write(socket, prompt, strlen(prompt)) < 0)
    fatal("ERROR writing prompt");

    while (1) {
        // Комада передачи файла
        char peek[6] = {0};
        // Читаем первые байты для опознания
        ssize_t n = recv(socket, peek, 5, MSG_PEEK);
        if (n < 0) fatal("peek");
        if (n == 5 && strncmp(peek, "FILE ", 5) == 0) {
            if (recv_file(socket) < 0) {
                write(socket, "Error receiving file\n", 21);
            }
            continue;
        }        

        // Получаем ответ клиента
        ssize_t bytes = read(socket, buff, sizeof(buff) - 1);
        if (bytes <= 0) {
            break;
        }
        buff[bytes] = '\0';

        // Команда выхода
        if (strncmp(buff, EXIT_MSG, sizeof(EXIT_MSG) - 1) == 0) {
            const char *bye = "Bye!\n";
            write(socket, bye, strlen(bye));
            break;
        }

        // Разбираем вход: [команда, арг1, арг2]
        if (parse_input(buff, tokens, MAX_TOKENS) != 3) {
            const char *err = "Неверный формат. Используйте: <команда> <число> <число>\n";
            write(socket, err, strlen(err));
            continue;
        }

        // Определяем операцию
        int choice = 0;
        if (strcmp(tokens[0], "sum") == 0) choice = 1;
        else if (strcmp(tokens[0], "sub") == 0) choice = 2;
        else if (strcmp(tokens[0], "mult") == 0) choice = 3;
        else if (strcmp(tokens[0], "div_op") == 0) choice = 4;
        else {
            const char *err = "Неизвестная команда. Повторите ввод.\n";
            write(socket, err, strlen(err));
            continue;
        }

        double a = atof(tokens[1]);
        double b = atof(tokens[2]);
        double result = select_op(choice)(a, b);

        int len;
        if (isnan(result)) {
            len = snprintf(buff, sizeof(buff), "Ошибка: Деление на нольы\n");
        } else {
            const char *fmt = (choice == 1 ? "Сумма: %lf\n" :
                               choice == 2 ? "Разность: %lf\n" :
                               choice == 3 ? "Умнождение: %lf\n" :
                                             "Деление: %lf\n");
            len = snprintf(buff, sizeof(buff), fmt, result);
        }

        if (len < 0) fatal("snprintf");
        if (write(socket, buff, len) < 0) fatal("write result");
    }
}

