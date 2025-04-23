#include <stdio.h>
#include "calculator.h"

int main() {
    int choice;

    printf("Работа: выбрать операцию -> ввести числа -> завершить аргументы вводом конечного маркера (-9999)\n");

    do {
        printf("\n=== Калькулятор ===\n");
        printf("Выберите операцию:\n");
        printf("1. Сумма\n");
        printf("2. Вычитание\n");
        printf("3. Умножение\n");
        printf("4. Деление\n");
        printf("0. Выход\n");
        printf("Ваш выбор: ");
        scanf("%d", &choice);
        
        if (choice == 0) {
            break;
        }

        double (*operation)(double, ...) = select_op(choice);
        if (operation == NULL) {
            printf("Неверный выбор операции!\n");
            continue;
        }

        double nums[NUMBERS_LEN];
        int count = 0;
        double x;
        printf("Введите числа (для завершения введите %lf):\n", END_MARKER);
        while (scanf("%lf", &x) == 1 && x != END_MARKER && count < NUMBERS_LEN) {
            nums[count++] = x;
        }

        if (count == 0) {
            printf("Нет чисел для выполнения операции.\n");
            continue;
        }

        // Обработка количества введённых чисел.
        double result;
        switch (count) {
            case 1:
                result = operation(nums[0], END_MARKER);
                break;
            case 2:
                result = operation(nums[0], nums[1], END_MARKER);
                break;
            case 3:
                result = operation(nums[0], nums[1], nums[2], END_MARKER);
                break;
            case 4:
                result = operation(nums[0], nums[1], nums[2], nums[3], END_MARKER);
                break;
            case 5:
                result = operation(nums[0], nums[1], nums[2], nums[3], nums[4], END_MARKER);
                break;
            default:
                printf("Введено слишком много чисел для данного примера.\n");
                continue;
        }

        // Вывод результата с соответствующей меткой
        switch (choice) {
            case 1:
                printf("Сумма: %lf\n", result);
                break;
            case 2:
                printf("Разность: %lf\n", result);
                break;
            case 3:
                printf("Произведение: %lf\n", result);
                break;
            case 4:
                if (result == END_MARKER)
                    break;
                printf("Частное: %lf\n", result);
                break;
            default:
                break;
        }
    } while (choice != 0);
    printf("Выход из программы.\n");
    return 0;
}
