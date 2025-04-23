#include "permissions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Вывод битового представления числа с заданным числом бит
void print_binary(int number, int bits) {
    printf("Права доступа (битовое): ");
    for (int i = bits - 1; i >= 0; i--) {
        printf("%d", (number >> i) & 1);
    }
    printf("\n");
}

// Целочисленное возведение в степень
int int_pow(int base, int exponent) {
    int result = 1;
    for (int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

// Преобразование буквенного представления прав (например, "rwxr-xr--") в число в виде восьмеричного представления.
// Если строка короче 9 символов – выводится сообщение об ошибке.
int literal_to_octal(char* literal) {
    if (strlen(literal) < 9) {
        fprintf(stderr, "Ошибка: Недостаточная длина строкового представления прав.\n");
        return -1;
    }
    int octal = 0;
    for (int i = 0; i < 3; i++) {
        int digit = 0;
        if (literal[i * 3] == 'r')
            digit += 4;
        if (literal[i * 3 + 1] == 'w')
            digit += 2;
        if (literal[i * 3 + 2] == 'x')
            digit += 1;
        // Собираем число: digit * 10^(2-i)
        octal += digit * int_pow(10, 2 - i);
    }
    return octal;
}

// Преобразование строки с восьмеричным представлением (например, "754") в битовую маску прав доступа.
int octal_str_to_perm(char* octal_str) {
    int perm = 0;
    for (size_t i = 0; i < strlen(octal_str); i++) {
        int digit = octal_str[i] - '0';
        if (digit < 0 || digit > 7) {
            fprintf(stderr, "Ошибка: Некорректная цифра в восьмеричном представлении.\n");
            return -1;
        }
        perm = (perm << 3) | digit;
    }
    return perm;
}

// Вывод прав доступа в цифровом (octal), буквенном (как в ls -l) и битовом виде.
void print_permissions(int perm) {
    printf("Права доступа (цифровое): %o\n", perm);
    printf("Права доступа (буквенное): %c%c%c%c%c%c%c%c%c\n",
           (perm & S_IRUSR) ? 'r' : '-',
           (perm & S_IWUSR) ? 'w' : '-',
           (perm & S_IXUSR) ? 'x' : '-',
           (perm & S_IRGRP) ? 'r' : '-',
           (perm & S_IWGRP) ? 'w' : '-',
           (perm & S_IXGRP) ? 'x' : '-',
           (perm & S_IROTH) ? 'r' : '-',
           (perm & S_IWOTH) ? 'w' : '-',
           (perm & S_IXOTH) ? 'x' : '-');
    print_binary(perm, 9);
}

// Получение информации о файле с использованием stat.
// Выводится размер файла и права доступа.
int print_file_info(char *filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("Ошибка вызова stat");
        return -1;
    }
    printf("Размер файла: %ld байт\n", st.st_size);
    int perm = st.st_mode & 0777;
    print_permissions(perm);
    return 0;
}

// Модификация прав доступа на основе команды модификации (например, "u+x", "g-w", "o=r").
int modify_permissions(int current_perm, char *cmd) {
    
    // Если команда начинается с цифры, считаем, что задан режим в виде числа
    if (cmd[0] >= '0' && cmd[0] <= '7') {
        int new_perm = octal_str_to_perm(cmd);
        if (new_perm == -1) {
            fprintf(stderr, "Неверный формат числовых прав.\n");
            return current_perm;
        }
        return new_perm;
    }
    
    int mod_mask = 0;
    int groups = 0; // Битовая маска: бит 001 - u, бит 010 - g, бит 100 - o
    int i = 0;
    
    // Чтение групп (u, g, o, a). Если указано 'a', то выбираются все группы.
    while (cmd[i] && cmd[i] != '+' && cmd[i] != '-' && cmd[i] != '=') {
        char ch = cmd[i];
        if (ch == 'a') {
            groups = 0b111;
        } else if (ch == 'u') {
            groups |= 0b001;
        } else if (ch == 'g') {
            groups |= 0b010;
        } else if (ch == 'o') {
            groups |= 0b100;
        }
        i++;
    }
    // Если группы не указаны – по умолчанию выбираем все.
    if (groups == 0) {
        groups = 0b111;
    }
    
    // Чтение оператора: +, - или =
    char op = cmd[i];
    i++; // Переход к символам прав

    // Формирование маски модификации по символам прав доступа
    while (cmd[i]) {
        char perm = cmd[i];
        if (perm == 'r') {
            if (groups & 0b001) mod_mask |= S_IRUSR;
            if (groups & 0b010) mod_mask |= S_IRGRP;
            if (groups & 0b100) mod_mask |= S_IROTH;
        } else if (perm == 'w') {
            if (groups & 0b001) mod_mask |= S_IWUSR;
            if (groups & 0b010) mod_mask |= S_IWGRP;
            if (groups & 0b100) mod_mask |= S_IWOTH;
        } else if (perm == 'x') {
            if (groups & 0b001) mod_mask |= S_IXUSR;
            if (groups & 0b010) mod_mask |= S_IXGRP;
            if (groups & 0b100) mod_mask |= S_IXOTH;
        }
        i++;
    }
    
    // Применение операции модификации
    switch (op) {
        case '+':
            current_perm |= mod_mask;
            break;
        case '-':
            current_perm &= ~mod_mask;
            break;
        case '=':
            // Сброс прав для выбранных групп и установка новых
            if (groups & 0b001)
                current_perm &= ~(S_IRUSR | S_IWUSR | S_IXUSR);
            if (groups & 0b010)
                current_perm &= ~(S_IRGRP | S_IWGRP | S_IXGRP);
            if (groups & 0b100)
                current_perm &= ~(S_IROTH | S_IWOTH | S_IXOTH);
            current_perm |= mod_mask;
            break;
        default:
            fprintf(stderr, "Неизвестная операция: %c\n", op);
            break;
    }
    
    return current_perm;
}

// Обработка пользовательского ввода прав доступа. Если первым символом является цифра – считаем ввод восьмеричным,
// если буква – буквенным представлением.
int process_permission_input(char* input) {
    if (!input || strlen(input) == 0) {
        fprintf(stderr, "Пустой ввод\n");
        return -1;
    }
    char first = input[0];
    int perm = -1;
    if (first >= '0' && first <= '7') {
        perm = octal_str_to_perm(input);
    } else if (first == 'r' || first == 'w' || first == 'x' || first == '-') {
        int octal = literal_to_octal(input);
        if (octal < 0)
            return -1;
        // Преобразуем полученное число в строку для дальнейшей конвертации
        char octal_str[5];
        snprintf(octal_str, sizeof(octal_str), "%d", octal);
        perm = octal_str_to_perm(octal_str);
    } else {
        fprintf(stderr, "Некорректный формат ввода.\n");
    }
    return perm;
}
