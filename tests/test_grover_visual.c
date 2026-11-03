#include <stdio.h>
#include "quantum_core.h"
#include "quantum_register.h"
#include "test_common.h"

/* Grover de 2 qubits: una iteracion debe dar el estado marcado con P ~ 1, para cada objetivo. */
int main(void) {
    for (uint16_t target = 0; target < 4; target++) {
        quantum_register_t reg = init_register(2);
        apply_gate_h(&reg, 0);
        apply_gate_h(&reg, 1);
        apply_oracle(&reg, target);
        apply_diffuser(&reg);
        assert_norm(&reg);
        CHECK(reg_prob(&reg, target) >= 0.99, "objetivo %u: P = %.4f", target, reg_prob(&reg, target));
    }
    printf("test_grover_visual: OK\n");
    return 0;
}
