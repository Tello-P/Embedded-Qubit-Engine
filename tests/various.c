#include <stdio.h>
#include <stdint.h>
#include "fixed_point.h"

void test_fp_mul_edges(void) {
    struct { int16_t a; int16_t b; int16_t expected; const char* desc; } cases[] = {
        { 16383,  16383,  16383, "+max * +max → casi +1" },
        {-16383,  16383, -16383, "-max * +max → casi -1" },
        { 16383, -16383, -16383, "+max * -max → casi -1" },
        {-16383, -16383,  16383, "-max * -max → casi +1" },
        { 16384,  16384,  16383, "+1.00006 * +1.00006 → saturado a +1" }, // si permites 16384
        {     0,  16383,      0, "zero * max = 0" },
        {  8192,   8192,   4096, "0.5 * 0.5 = 0.25" },
    };

    printf("\nTesting fp_mul edge cases:\n");
    for (int i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
        int16_t res = fp_mul(cases[i].a, cases[i].b);
        printf("%6d * %6d = %6d  | expected ~%6d  (%s) %s\n",
               cases[i].a, cases[i].b, res, cases[i].expected,
               cases[i].desc, (res == cases[i].expected || abs(res - cases[i].expected) <= 1) ? "OK" : "FAIL");
    }
}void test_hadamard_zero(void) {
    qubit_t q = { .alpha = {Q14_ONE, 0}, .beta = {0, 0} };  // |0⟩

    apply_gate_h(&q);

    // Esperado: alpha ≈ beta ≈ 0.7071 → 11585 en Q14
    int16_t expected = 11585;
    int err_a = abs(q.alpha.real - expected);
    int err_b = abs(q.beta.real  - expected);

    printf("\nHadamard on |0⟩:\n");
    printf("alpha = %6d + %6di  → error %d\n", q.alpha.real, q.alpha.imag, err_a);
    printf("beta  = %6d + %6di  → error %d\n", q.beta.real,  q.beta.imag,  err_b);
    printf("%s (tolerance < 163 ≈ 0.01)\n", (err_a < 163 && err_b < 163) ? "OK" : "FAIL");
}void apply_gate_cnot(qubit_t *control, qubit_t *target) {
    if (control->beta.real == Q14_ONE && control->beta.imag == 0) {  // si control está en |1⟩
        complex_q14_t temp = target->alpha;
        target->alpha = target->beta;
        target->beta  = temp;
    }
    // Nota: esto es solo para base computacional → no maneja superposiciones correctamente
    // Para versión completa necesitas 2-qubit statevector o más qubits
}void test_measure_plus_10000(void) {
    qubit_t q;
    apply_gate_h(&q);           // de |0> → |+>

    int count_0 = 0;
    const int TRIALS = 10000;

    // Fijar semilla para reproducibilidad
    rng_state = 0x123456789ABCDEF0ULL;   // cualquier valor fijo

    for (int i = 0; i < TRIALS; i++) {
        qubit_t copy = q;               // no queremos modificar el original
        int result = measure(&copy);
        if (result == 0) count_0++;
    }

    float p0 = (float)count_0 / TRIALS;
    printf("\n10000 measures of |+⟩ state:\n");
    printf("P(0) = %.4f  P(1) = %.4f  → %s\n", p0, 1.0f - p0,
           (p0 > 0.48f && p0 < 0.52f) ? "OK (dentro ~2% desviación)" : "FAIL (sesgo grande)");
}int main(void) {
    printf("=== Quantum fixed-point Q14 tests ===\n\n");

    test_fp_mul_edges();
    test_hadamard_zero();
    // test_bell_magnitudes();     // cuando tengas CNOT + 2 qubits
    test_measure_plus_10000();

    printf("\nTests finalizados.\n");
    return 0;
}
