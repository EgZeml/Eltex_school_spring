#include <stdio.h>
#include <string.h>

// Очищение потока ввода
void flush_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Ввод данных с удалением экранирования
void input_string(char* string, int size) {
    fgets(string, size, stdin);
    string[strcspn(string, "\n")] = '\0';
}