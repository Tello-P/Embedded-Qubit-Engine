#include <stdio.h>
#include "quantum_register.h"
#include "quantum_core.h"
#include "test_common.h"

/* Superposicion y GHZ con todos los qubits que admite la compilacion (MAX_QUBITS). */
int main(void) {
    for (uint8_t n = 1; n <= MAX_QUBITS; n++) {
        quantum_register_t reg = init_register(n);
        for (uint8_t q = 0; q < n; q++) apply_gate_h(&reg, q);
        assert_norm(&reg);
        for (uint16_t i = 0; i < reg.dim; i++)
            CHECK(abs((int)(reg_prob(&reg, i) * reg.dim * 1000) - 1000) < 30,
                  "n=%u: P(%u) = %.5f, esperada %.5f", n, i, reg_prob(&reg, i), 1.0 / reg.dim);

        reg = init_register(n);
        apply_gate_h(&reg, 0);
        for (uint8_t q = 1; q < n; q++) apply_gate_cnot(&reg, 0, q);
        assert_norm(&reg);
        if (n > 1) {
            CHECK(reg_prob(&reg, 0) > 0.49 && reg_prob(&reg, 0) < 0.51, "GHZ n=%u: P(0..0)", n);
            CHECK(reg_prob(&reg, reg.dim - 1) > 0.49 && reg_prob(&reg, reg.dim - 1) < 0.51, "GHZ n=%u: P(1..1)", n);
        }
    }
    printf("test_multiple_qubits: OK (hasta %d qubits)\n", MAX_QUBITS);
    return 0;
}
