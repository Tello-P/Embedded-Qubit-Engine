#include <stdio.h>
#include <stdint.h>
#include "quantum_register.h"
#include "fixed_point.h"
#include "quantum_core.h"

void print_register_status(const quantum_register_t *reg) {
    if (reg->state == NULL || reg->dim == 0) {
        printf("Invalid Register\n");
        return;
    }
    printf("Register (%d Qubits):\n", reg->num_qubits);
    for (uint16_t i = 0; i < reg->dim; i++) {
        printf("  |%d%d%s⟩: ", (i >> 1) & 1, i & 1, reg->num_qubits > 2 ? ".." : "");
        float re = (float)reg->state[i].real / Q14_ONE;
        float im = (float)reg->state[i].imag / Q14_ONE;
        printf("%+6.3f %+6.3fi | Prob: %3d%%\n", re, im, 
               (complex_mag_sq(reg->state[i]) * 100) / Q14_ONE);
    }
    printf("----------------------------------\n");
}

void apply_gate_to_reg(quantum_register_t *reg, uint8_t target_qubit, void (*gate)(qubit_t*)) {
    uint16_t stride = 1 << target_qubit;
    for (uint16_t i = 0; i < reg->dim; i++) {
        if (!(i & stride)) {
            uint16_t i0 = i;
            uint16_t i1 = i | stride;
            qubit_t temp = {reg->state[i0], reg->state[i1]};
            gate(&temp);
            reg->state[i0] = temp.alpha;
            reg->state[i1] = temp.beta;
        }
    }
}

int main(void) {
    printf("=== Starting Quantum Core Test (Static Allocation) ===\n\n");

    quantum_register_t reg = init_register(2);
    printf("Initial State (|00>):\n");
    print_register_status(&reg);

    printf("Applying X-Gate to Qubit 0...\n");
    apply_gate_to_reg(&reg, 0, apply_gate_x);
    print_register_status(&reg);

    printf("Applying H-Gate to Qubit 1 (Superposition)...\n");
    apply_gate_to_reg(&reg, 1, apply_gate_h);
    print_register_status(&reg);

    printf("Measuring Qubit 0 (should be 1 with 100%% probability)...\n");
    qubit_t q_final = {reg.state[1], reg.state[3]}; // Extracting amplitudes for Qubit 0
    int result = measure(&q_final);
    printf("Measured value: %d\n", result);

    return 0;
}
