#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "quantum_core.h"
#include "quantum_register.h"
#include "test_common.h"

/* Grover con 3 qubits (2 iteraciones). P teorica = sin^2(5*asin(1/sqrt(8))) = 0.9453.
   Se comprueba para los 8 objetivos (o solo uno si se pasa 1-8 por argv). */
static void run_target(uint16_t target) {
    quantum_register_t reg = init_register(3);
    for (uint8_t q = 0; q < 3; q++) apply_gate_h(&reg, q);
    for (int iter = 0; iter < 2; iter++) {
        apply_oracle(&reg, target);
        apply_diffuser(&reg);
        assert_norm(&reg);
    }
    double theory = pow(sin(5 * asin(1 / sqrt(8.0))), 2);
    double p = reg_prob(&reg, target);
    CHECK(fabs(p - theory) < 0.02, "objetivo %u: P = %.4f, teorica %.4f", target, p, theory);
    for (uint16_t i = 0; i < reg.dim; i++)
        if (i != target) CHECK(reg_prob(&reg, i) < p, "el estado %u supera al marcado", i);
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        int input = atoi(argv[1]);
        CHECK(input >= 1 && input <= 8, "uso: %s [1-8]", argv[0]);
        run_target((uint16_t)(input - 1));
    } else {
        for (uint16_t t = 0; t < 8; t++) run_target(t);
    }
    printf("test_grover: OK\n");
    return 0;
}
