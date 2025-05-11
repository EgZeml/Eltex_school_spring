#include "math.h"
#include <math.h>
#include <stdlib.h>

double sum(double a, double b) {
    return a + b;
}

double sub(double a, double b) {
    return a - b;
}

double mult(double a, double b) {
    return a * b;
}

double div_op(double a, double b) {
    return (b == 0.0) ? NAN : a / b;
}

double (*select_op(int choice))(double, double) {
    switch (choice) {
        case 1: return sum;
        case 2: return sub;
        case 3: return mult;
        case 4: return div_op;
        default: return NULL;
    }
}
