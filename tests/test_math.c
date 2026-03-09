#include <stdio.h>
#include "fixed_point.h"

int main() {
    printf("--- Validando Motor Matemático Q1.14 ---\n");
    
    int16_t res = fp_mul(Q14_INV_SQRT2, Q14_INV_SQRT2);
    
    printf("Test 1/sqrt(2) * 1/sqrt(2):\n");
    printf("Resultado: %d (Esperado: 8192)\n", res);

    if (res >= 8191 && res <= 8193) {
        printf("STATUS: SUCCESS\n");
    } else {
        printf("STATUS: FAILURE\n");
    }

    return 0;
}
