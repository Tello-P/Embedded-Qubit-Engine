#include <stdio.h>
#include "quantum_core.h"
#include "quantum_register.h"

void print_register(quantum_register_t *reg) {
    for (uint16_t i = 0; i < reg->dim; i++) {
        printf("|%d%d>: %d + %di\n", (i >> 1) & 1, i & 1, reg->state[i].real, reg->state[i].imag);
    }
}

int main() {
    quantum_register_t reg = init_register(2);

    apply_gate_h(&reg, 0);

    apply_gate_cnot(&reg, 0, 1);

    print_register(&reg);

    return 0;
}
