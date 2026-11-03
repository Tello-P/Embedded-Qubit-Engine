#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "fixed_point.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static int failures = 0;

void print_test_result(const char* test_name, bool passed) {
    if (!passed) failures++;
    if (passed) {
        printf("%s: " ANSI_COLOR_GREEN "PASSED" ANSI_COLOR_RESET "\n", test_name);
    } else {
        printf("%s: " ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET "\n", test_name);
    }
}

void test_fp_mul() {
    int16_t a = 8192;
    int16_t b = 8192;
    int16_t res = fp_mul(a, b);
    print_test_result("Multiplicacion Simple (0.5 * 0.5)", res == 4096);

    res = fp_mul(Q14_INV_SQRT2, Q14_INV_SQRT2);
    print_test_result("Precision Hadamard (1/sqrt2^2)", res >= 8191 && res <= 8193);
}

void test_complex_math() {
    complex_q14_t c1 = {Q14_ONE, 0};
    complex_q14_t c2 = {0, Q14_ONE};
    complex_q14_t res = complex_mul(c1, c2);
    
    print_test_result("Multiplicacion Compleja (Rotacion i)", (res.real == 0 && res.imag == Q14_ONE));

    complex_q14_t h = {Q14_INV_SQRT2, Q14_INV_SQRT2};
    int16_t mag_sq = complex_mag_sq(h);
    print_test_result("Módulo al cuadrado (Normalización)", mag_sq >= 16383 && mag_sq <= 16385);
}

/* fp_mul debe coincidir con la multiplicacion real redondeada (error <= 1 LSB)
   en una rejilla que incluye negativos y los extremos de Q1.14. */
void test_fp_mul_grid() {
    int worst = 0;
    for (int a = -Q14_ONE; a <= Q14_ONE; a += 997) {
        for (int b = -Q14_ONE; b <= Q14_ONE; b += 991) {
            double exact = (double)a * b / Q14_ONE;
            int err = abs((int)fp_mul((int16_t)a, (int16_t)b) - (int)(exact + (exact < 0 ? -0.5 : 0.5)));
            if (err > worst) worst = err;
        }
    }
    printf("fp_mul: error maximo en rejilla = %d LSB\n", worst);
    print_test_result("fp_mul vs double (<= 1 LSB)", worst <= 1);
}

void test_mag_sq_grid() {
    int worst = 0;
    for (int re = -Q14_ONE; re <= Q14_ONE; re += 1499) {
        for (int im = -Q14_ONE; im <= Q14_ONE; im += 1493) {
            double exact = ((double)re * re + (double)im * im) / Q14_ONE;
            if (exact > 32767) continue; /* fuera de rango int16, no es un estado valido */
            int err = abs((int)complex_mag_sq((complex_q14_t){(int16_t)re, (int16_t)im}) - (int)exact);
            if (err > worst) worst = err;
        }
    }
    print_test_result("complex_mag_sq vs double (<= 1 LSB)", worst <= 1);
}

int main() {
    printf("\n=== AVR-QUANTUM-CORE: SUITE DE PRUEBAS FASE 1 ===\n\n");
    
    test_fp_mul();
    test_complex_math();
    test_fp_mul_grid();
    test_mag_sq_grid();
    
    printf("\n================================================\n");
    return failures ? 1 : 0;
}
