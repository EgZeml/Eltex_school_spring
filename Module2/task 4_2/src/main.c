#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "queue.h"

// Меню
void print_menu() {
    printf("\nМеню:\n");
    printf("1. Сгенерировать 5 элементов\n");
    printf("2. Достать элемент с самым высоким приоритетом (наименьшее число)\n");
    printf("3. Достать элемент с заданным приоритетом\n");
    printf("4. Достать элемент с приоритетом не ниже заданного\n");
    printf("0. Выход\n");
    printf("Ваш выбор: ");
}

int main() {
    PriorityQueue queue = {NULL, NULL};
    srand(time(NULL));

    int choice;
    do {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Некорректный ввод. Попробуйте снова.\n");
            while(getchar() != '\n');
            continue;
        }
        Node* extracted = NULL;
        int priority;
        switch (choice) {
            case 1:
                for (int i = 0; i < 5; i++) {
                    int pr = rand() % 256;
                    int data = rand() % 1000;
                    enqueue(&queue, pr, data);
                }
                printf("5 элементов сгенерированы и добавлены в очередь.\n");
                print_queue(&queue);
                break;
            case 2:
                extracted = dequeue(&queue);
                if (extracted) {
                    printf("Извлечён элемент с самым высоким приоритетом: [Приоритет: %d, Данные: %d]\n", 
                           extracted->priority, extracted->data);
                    free(extracted);
                } else {
                    printf("Очередь пуста!\n");
                }
                print_queue(&queue);
                break;
            case 3:
                printf("Введите требуемый приоритет: ");
                scanf("%d", &priority);
                extracted = extract_by_priority(&queue, priority);
                if (extracted) {
                    printf("Извлечён элемент с приоритетом %d: [Приоритет: %d, Данные: %d]\n", 
                           priority, extracted->priority, extracted->data);
                    free(extracted);
                } else {
                    printf("Элемент с приоритетом %d не найден!\n", priority);
                }
                print_queue(&queue);
                break;
            case 4:
                printf("Введите минимальный порог приоритета: ");
                scanf("%d", &priority);
                extracted = extract_by_min_priority(&queue, priority);
                if (extracted) {
                    printf("Извлечён элемент с приоритетом не ниже %d: [Приоритет: %d, Данные: %d]\n", 
                           priority, extracted->priority, extracted->data);
                    free(extracted);
                } else {
                    printf("Нет элементов с приоритетом не ниже %d!\n", priority);
                }
                print_queue(&queue);
                break;
            case 0:
                printf("Выход из программы.\n");
                while ((extracted = dequeue(&queue)) != NULL) {
                    free(extracted);
                }
                break;
            default:
                printf("Неверный выбор. Попробуйте снова.\n");
                break;
        }
    } while (choice != 0);
    
    return 0;
}
