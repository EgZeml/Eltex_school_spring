#include <stdio.h>
#include "calculator.h"

double (*select_op(int choice)) (double, double) {
    switch (choice) {
        case 1: return sum;
        case 2: return sub;
        case 3: return mult;
        case 4: return div_op;
        default: return NULL;
    }
}
