#include <stdio.h>
#include <stdint.h>
#include "fixed_point.h"
#include "quantum_core.h"
#include "test_common.h"

static void test_gate_x(void) {
    quantum_register_t r = init_register(1);
    apply_gate_x(&r, 0);
    CHECK(r.state[0].real == 0 && r.state[1].real == Q14_ONE, "X|0> != |1>");
    apply_gate_x(&r, 0);
    CHECK(r.state[0].real == Q14_ONE && r.state[1].real == 0, "XX != I");
    assert_norm(&r);
}

static void test_gate_h(void) {
    quantum_register_t r = init_register(1);
    apply_gate_h(&r, 0);
    CHECK(abs(r.state[0].real - Q14_INV_SQRT2) <= 1 && abs(r.state[1].real - Q14_INV_SQRT2) <= 1,
          "H|0> = (%d, %d)", r.state[0].real, r.state[1].real);
    assert_norm(&r);
    apply_gate_h(&r, 0);
    CHECK(abs(r.state[0].real - Q14_ONE) <= 4 && abs(r.state[1].real) <= 4,
          "HH|0> = (%d, %d), esperado (16384, 0)", r.state[0].real, r.state[1].real);
}

static void test_gate_z(void) {
    quantum_register_t r = init_register(1);
    apply_gate_x(&r, 0);
    apply_gate_z(&r, 0);
    CHECK(r.state[1].real == -Q14_ONE, "Z|1> debe ser -|1>");
    assert_norm(&r);
}

static void test_measurement_statistics(void) {
    seed_quantum_rng(42);
    int ones = 0, total = 2000;
    for (int i = 0; i < total; i++) {
        quantum_register_t r = init_register(1);
        apply_gate_h(&r, 0);
        ones += measure_qubit(&r, 0);
        assert_norm(&r);
    }
    /* 5 sigma de Binomial(2000, 0.5) = +-112 */
    CHECK(ones > 1000 - 112 && ones < 1000 + 112, "unos = %d de %d", ones, total);
}

static void test_measure_deterministic(void) {
    quantum_register_t r = init_register(1);
    CHECK(measure_qubit(&r, 0) == 0, "medir |0> debe dar 0");
    assert_norm(&r);   /* regresion: la medida de un qubit definido daba norma 1.41 */
    apply_gate_x(&r, 0);
    CHECK(measure_qubit(&r, 0) == 1, "medir |1> debe dar 1");
    assert_norm(&r);
}

int main(void) {
    test_gate_x();
    test_gate_h();
    test_gate_z();
    test_measure_deterministic();
    test_measurement_statistics();
    printf("test_quantum: OK\n");
    return 0;
}
