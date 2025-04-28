#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h> 

#define MAX_LINE_LEN 256
#define MAX_ARGS     16

// Реализовать в makefile добавление папки в путь: export PATH=$PATH:~/myapps

// Разбивает строку `line` на токены и заполняет argv[0..]
// Возвращает число аргументов
int parse_command(char *line, char *argv[], int max_args) {
    int argc = 0;
    char *token = strtok(line, " \t");
    while (token != NULL && argc < max_args - 1) {
        argv[argc] = token;
        token = strtok(NULL, " \t");
        argc++;
    }
    argv[argc] = NULL;
    return argc;
}

int main() {
    char line[MAX_LINE_LEN];

    while (1) {

        // 1) Показываем приглашение
        printf("shell> ");
        fflush(stdout);

        // 2) Считываем строку
        if (!fgets(line, sizeof(line), stdin)) {
            // — Ctrl-D или ошибка
            if (feof(stdin)) {
                putchar('\n');
            } else {
                perror("fgets");
            }
            break;
        }

        // 3) Убираем '\n'
        line[strcspn(line, "\n")] = '\0';

        // 4) Проверяем команду exit
        if (strcmp(line, "exit") == 0) {
            break;
        }
        if (line[0] == '\0') {
            continue;  // пустая строка — заново
        }

        // 5) Парсим на argv[]
        // Заменяем пробельные символы на '\0'
        char *argv[MAX_ARGS];
        int argc = parse_command(line, argv, MAX_ARGS);
        if (argc == 0) {
            continue;
        }

        // 6) Создаём процесс
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            // дочерний
            execvp(argv[0], argv);
            // Если сюда зашли — exec не отработал
            fprintf(stderr, "execvp(%s): %s\n", argv[0], strerror(errno));
            _exit(EXIT_FAILURE); // Немедленно завершить
        } else {
            // родитель ждёт завершения ребёнка
            int status;
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
            }
        }
    }

    return 0;
}
