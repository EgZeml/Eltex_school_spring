#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <sys/stat.h>

// Максимальная длина имени файла
#define FILE_NAME_LEN 32

// Структура для хранения информации об изменённых файлах
typedef struct {
    char name[FILE_NAME_LEN];
    int permission;
} FilePermission;

// Функции для работы с правами доступа
// Вывод битового представления числа (число, количество бит)
void print_binary(int number, int bits);

// Целочисленное возведение в степень (base^exponent)
int int_pow(int base, int exponent);

// Преобразование буквенного представления (например, "rwxr-xr--") в число в восьмеричном виде
int literal_to_octal(char* literal);

// Преобразование строки с восьмеричным представлением (например, "754") в битовую маску
int octal_str_to_perm(char* octal_str);

// Вывод прав доступа в трёх форматах: цифровое (восьмеричное), буквенное и битовое
void print_permissions(int perm);

// Получение информации о файле (размер и права доступа) с использованием stat
int print_file_info(char *filename);

// Модификация прав доступа по команде (например, "u+x", "g-w", "o=r")
int modify_permissions(int current_perm, char *cmd);

// Обработка пользовательского ввода прав доступа (определение типа ввода и преобразование)
int process_permission_input(char* input);

#endif // PERMISSIONS_H
