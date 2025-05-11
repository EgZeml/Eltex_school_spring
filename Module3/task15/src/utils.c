#include "utils.h"
#include "math.h"
#include <string.h>
#include <math.h>

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
        "  quit   - выход\n";

    // Просим команду
    if (write(socket, prompt, strlen(prompt)) < 0)
    fatal("ERROR writing prompt");

    while (1) {
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
            len = snprintf(buff, sizeof(buff), "Error: Division by zero\n");
        } else {
            const char *fmt = (choice == 1 ? "Сумма: %lf\n" :
                               choice == 2 ? "Разница: %lf\n" :
                               choice == 3 ? "Умножение: %lf\n" :
                                             "Деление: %lf\n");
            len = snprintf(buff, sizeof(buff), fmt, result);
        }

        if (len < 0) fatal("snprintf");
        if (write(socket, buff, len) < 0) fatal("write result");
    }
}
