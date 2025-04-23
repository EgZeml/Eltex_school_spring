#include <stdio.h>
#include "calculator.h"

double sum(double first, ...) {
    va_list args;
    double total = first;
    va_start(args, first);

    double num;
    while ((num = va_arg(args, double)) != END_MARKER) {
        total += num;
    }

    va_end(args);
    return total;
}

double sub(double first, ...) {
    va_list args;
    double total = first;
    va_start(args, first);

    double num;
    while ((num = va_arg(args, double)) != END_MARKER) {
        total -= num;
    }

    va_end(args);
    return total;
}

double mult(double first, ...) {
    va_list args;
    double total = first;
    va_start(args, first);

    double num;
    while ((num = va_arg(args, double)) != END_MARKER) {
        total *= num;
    }

    va_end(args);
    return total;
}

double div_op(double first, ...) {
    va_list args;
    double total = first;
    va_start(args, first);

    double num;
    double final_div = 1.0;
    while ((num = va_arg(args, double)) != END_MARKER) {
        if (num == 0) {
            printf("Делить на ноль нельзя\n");
            va_end(args);
            return END_MARKER;
        }
        final_div *= num;
    }
    total /= final_div;
    va_end(args);
    return total;
}

double (*select_op(int choice)) (double, ...) {
    switch (choice) {
        case 1: return sum;
        case 2: return sub;
        case 3: return mult;
        case 4: return div_op;
        default: return NULL;
    }
}
