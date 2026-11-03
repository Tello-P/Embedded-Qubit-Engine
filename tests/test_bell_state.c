#include <stdio.h>
#include "quantum_core.h"
#include "quantum_register.h"
#include "test_common.h"

int main(void) {
    quantum_register_t reg = init_register(2);
    apply_gate_h(&reg, 0);
    apply_gate_cnot(&reg, 0, 1);

    assert_norm(&reg);
    CHECK(abs(reg.state[0].real - Q14_INV_SQRT2) <= 1, "|00> = %d", reg.state[0].real);
    CHECK(abs(reg.state[3].real - Q14_INV_SQRT2) <= 1, "|11> = %d", reg.state[3].real);
    CHECK(reg.state[1].real == 0 && reg.state[2].real == 0, "|01>/|10> deben ser 0");

    /* Colapso: tras medir el qubit 0, ambos qubits deben coincidir y la norma ser 1.
       Se repite con varias semillas para ver ambos resultados. */
    int seen[2] = {0, 0};
    for (uint64_t seed = 1; seed <= 40; seed++) {
        seed_quantum_rng(seed);
        reg = init_register(2);
        apply_gate_h(&reg, 0);
        apply_gate_cnot(&reg, 0, 1);
        int m = measure_qubit(&reg, 0);
        seen[m]++;
        assert_norm(&reg);
        CHECK(reg_prob(&reg, m ? 3 : 0) > 0.99, "seed %llu: tras medir %d el estado no colapso", (unsigned long long)seed, m);
        CHECK(measure_qubit(&reg, 1) == m, "seed %llu: qubit 1 no correlacionado", (unsigned long long)seed);
    }
    CHECK(seen[0] > 0 && seen[1] > 0, "solo se observo un resultado (%d/%d)", seen[0], seen[1]);
    printf("test_bell_state: OK (resultados 0:%d 1:%d)\n", seen[0], seen[1]);
    return 0;
}
