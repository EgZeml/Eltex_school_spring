#include "contacts_types.h"
#include "contacts_logic.h"
#include "contacts_ui.h"
#include <stdlib.h>

// ANSI escape-последовательности для цвета
#define COLOR_BLUE "\033[1;34m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_RED "\033[1;31m"
#define COLOR_RESET "\033[0m"

int read_int() {
    int num;
    while (scanf("%d", &num) != 1) {
        printf(COLOR_RED "Некорректный ввод. Попробуйте снова: " COLOR_RESET);
        flush_input(); // Очистка неправильного ввода
    }
    flush_input();
    return num;
}

void print_menu() {
    system("clear");

    // Вывод заголовка меню с цветом
    printf(COLOR_BLUE "========================================\n");
    printf("            Книга Контактов             \n");
    printf("========================================\n" COLOR_RESET);
    printf("1. Добавить контакт\n");
    printf("2. Редактировать контакт\n");
    printf("3. Удалить контакт\n");
    printf("4. Добавить тестовый контакт\n");
    printf("5. Вывести все контакты\n");
    printf("6. Вывести дерево\n");
    printf("0. Выход\n");
    printf(COLOR_BLUE "========================================\n" COLOR_RESET);
    printf("Выберите пункт меню: ");
}

int main(void) {
    Contacts_book book;
    book.next_id = 1;
    book.count = 0;
    book.root = NULL;
    int choice, id;
    Contact contact_tmp;
    Tree_node* node;

    do {
        print_menu();
        choice = read_int();

        switch (choice) {
            case 1:
                contact_tmp = add_contact_ui();
                if (add_logic(&book, &contact_tmp) == 0)
                    printf(COLOR_GREEN "Контакт добавлен.\n" COLOR_RESET);
                else
                    printf(COLOR_RED "Ошибка: книга заполнена или неверные данные.\n" COLOR_RESET);
                break;
            case 2:
                printf("Введите id контакта для редактирования: ");
                id = read_int();
                node = search(book.root, id);
                if (node == NULL) {
                    printf(COLOR_RED "Ошибка: контакт не найден.\n" COLOR_RESET);
                    break;
                }
                contact_tmp = edit_contact_ui(node);
                if (edit_logic(&book, id, &contact_tmp) == 0)
                    printf(COLOR_GREEN "Контакт изменен.\n" COLOR_RESET);
                else
                    printf(COLOR_RED "Ошибка при редактировании контакта.\n" COLOR_RESET);
                break;
            case 3:
                printf("Введите id контакта для удаления: ");
                id = read_int();
                if (delete_logic(&book, id) == 0)
                    printf(COLOR_GREEN "Контакт удалён.\n" COLOR_RESET);
                else
                    printf(COLOR_RED "Контакт не найден.\n" COLOR_RESET);
                break;
            case 4:
                contact_tmp = add_test_contact_ui();
                if (add_logic(&book, &contact_tmp) == 0)
                    printf(COLOR_GREEN "Контакт добавлен.\n" COLOR_RESET);
                else
                    printf(COLOR_RED "Ошибка: книга заполнена или неверные данные.\n" COLOR_RESET);
                break;
            case 5:
                inorder(book.root);
                break;
            case 6:
                print_tree(book.root, 0);
                break;
            case 0:
                printf("Выход из программы...\n");
                break;
            default:
                printf(COLOR_RED "Некорректный выбор. Попробуйте снова.\n" COLOR_RESET);
        }

        if (book.count % 5 == 0) {
            balance_tree(&book);
            printf("Дерево сбалансировано\n");
        }

        // Пауза для ознакомления с результатом
        printf("Нажмите Enter, чтобы продолжить...\n");
        getchar();
    } while (choice != 0);

    return 0;
}
