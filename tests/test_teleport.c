#include <stdio.h>
#include "quantum_core.h"
#include "quantum_register.h"
#include "test_common.h"

/* Teleporta el estado q0 = H X |0> = |-> a q2 y lo verifica en cada ejecucion. */
static void teleport_once(uint64_t seed, int *m0_out, int *m1_out) {
    seed_quantum_rng(seed);
    quantum_register_t reg = init_register(3);

    apply_gate_x(&reg, 0);
    apply_gate_h(&reg, 0);
    assert_norm(&reg);

    apply_gate_h(&reg, 1);
    apply_gate_cnot(&reg, 1, 2);
    apply_gate_cnot(&reg, 0, 1);
    apply_gate_h(&reg, 0);
    assert_norm(&reg);

    int m1 = measure_qubit(&reg, 1);
    int m0 = measure_qubit(&reg, 0);
    assert_norm(&reg);

    if (m1 == 1) apply_gate_x(&reg, 2);
    if (m0 == 1) apply_gate_z(&reg, 2);
    assert_norm(&reg);

    /* q2 debe ser |-> : H X lo devuelve a |0> con probabilidad 1 */
    apply_gate_h(&reg, 2);
    apply_gate_x(&reg, 2);
    assert_norm(&reg);

    double p_q2_is_1 = 0;
    for (uint16_t i = 0; i < reg.dim; i++) if (i & 4) p_q2_is_1 += reg_prob(&reg, i);
    CHECK(p_q2_is_1 < 0.01, "seed %llu (m0=%d m1=%d): P(q2=1) = %.4f, la teleportacion fallo",
          (unsigned long long)seed, m0, m1, p_q2_is_1);
    CHECK(measure_qubit(&reg, 2) == 0, "seed %llu: medida final != 0", (unsigned long long)seed);
    *m0_out = m0; *m1_out = m1;
}

int main(void) {
    int seen[4] = {0};
    for (uint64_t seed = 1; seed <= 100; seed++) {
        int m0, m1;
        teleport_once(seed, &m0, &m1);
        seen[m0 * 2 + m1]++;
    }
    for (int k = 0; k < 4; k++) CHECK(seen[k] > 0, "nunca se observo el sindrome %d (sesgo del RNG o de la medida)", k);
    printf("test_teleport: OK (sindromes %d/%d/%d/%d)\n", seen[0], seen[1], seen[2], seen[3]);
    return 0;
}
