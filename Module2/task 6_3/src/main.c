#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>

#define PLUGIN_DIR "./plugins"
#define MAX_OPS 10

typedef double (*operation_t)(double, double);

typedef struct {
    char name[256];       // имя операции
    operation_t operate;  // указатель на функцию операции
    void *handle;         // дескриптор динамической библиотеки
} Operation;

// Убирает префикс "lib" и суффикс ".so" из имени файла
void format_op_name(const char *filename, char *opName, size_t max_len) {
    const char *start = filename;
    if (strncmp(filename, "lib", 3) == 0) {
        start = filename + 3;
    }
    strncpy(opName, start, max_len - 1);
    opName[max_len - 1] = '\0';
    char *dot = strstr(opName, ".so");
    if (dot) {
        *dot = '\0';
    }
}

int main() {
    Operation ops[MAX_OPS];
    int op_count = 0;

    DIR *dir = opendir(PLUGIN_DIR);
    if (!dir) {
        perror("Невозможно открыть каталог с плагинами");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && op_count < MAX_OPS) {
        // пропускаем не-.so файлы
        if (!strstr(entry->d_name, ".so")) {
            continue;
        }

        // формируем полный путь к библиотеке
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", PLUGIN_DIR, entry->d_name);

        // загружаем библиотеку
        void *handle = dlopen(path, RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "Ошибка загрузки %s: %s\n", entry->d_name, dlerror());
            continue;
        }

        // ищем символ operate
        dlerror(); // сбрасываем предыдущие ошибки
        operation_t func = (operation_t)dlsym(handle, "operate");
        char *error = dlerror();
        if (error) {
            fprintf(stderr, "Ошибка поиска символа в %s: %s\n", entry->d_name, error);
            dlclose(handle);
            continue;
        }

        // сохраняем операцию
        format_op_name(entry->d_name, ops[op_count].name, sizeof(ops[op_count].name));
        ops[op_count].operate = func;
        ops[op_count].handle = handle;
        op_count++;
    }
    closedir(dir);

    if (op_count == 0) {
        fprintf(stderr, "Плагины не найдены.\n");
        return 1;
    }

    int choice;
    do {
        printf("\n=== Калькулятор ===\n");
        for (int i = 0; i < op_count; i++) {
            printf(" %d. %s\n", i + 1, ops[i].name);
        }
        printf(" 0. Выход\n");
        printf("Введите номер операции: ");
        if (scanf("%d", &choice) != 1) {
            printf("Неверный ввод. Повторите попытку.\n");
            int c; while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        if (choice == 0) {
            break;
        }
        if (choice < 1 || choice > op_count) {
            printf("Неверный выбор операции! Повторите попытку.\n");
            continue;
        }

        double a, b;
        printf("Введите первое число: ");
        scanf("%lf", &a);
        while (getchar() != '\n');
        printf("Введите второе число: ");
        scanf("%lf", &b);
        while (getchar() != '\n');

        // проверка деления на ноль
        if (strcmp(ops[choice - 1].name, "div") == 0 && b == 0.0) {
            printf("Ошибка: деление на ноль!\n");
            continue;
        }

        double result = ops[choice - 1].operate(a, b);
        printf("Результат %s: %lf\n", ops[choice - 1].name, result);

    } while (1);

    // закрываем все загруженные библиотеки
    for (int i = 0; i < op_count; i++) {
        dlclose(ops[i].handle);
    }

    printf("Выход из программы.\n");
    return 0;
}
