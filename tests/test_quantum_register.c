#include <stdio.h>
#include "quantum_register.h"
#include "test_common.h"

int main(void) {
    for (uint8_t n = 1; n <= MAX_QUBITS; n++) {
        quantum_register_t r = init_register(n);
        CHECK(r.num_qubits == n && r.dim == (1u << n), "n=%u dim=%u", n, r.dim);
        CHECK(r.state != NULL, "estado nulo");
        CHECK(r.state[0].real == Q14_ONE && r.state[0].imag == 0, "|0..0> debe valer 1");
        for (uint16_t i = 1; i < r.dim; i++)
            CHECK(r.state[i].real == 0 && r.state[i].imag == 0, "indice %u debe ser 0", i);
        assert_norm(&r);
    }
    /* casos invalidos: deben devolver un registro vacio, no uno parcial */
    quantum_register_t bad = init_register(0);
    CHECK(bad.dim == 0 && bad.state == NULL, "n=0 debe ser invalido");
    bad = init_register(MAX_QUBITS + 1);
    CHECK(bad.dim == 0 && bad.state == NULL, "n=MAX+1 debe ser invalido");
    bad = init_register(10);
    CHECK(bad.dim == 0 && bad.state == NULL, "n=10 debe ser invalido");
    printf("test_quantum_register: OK\n");
    return 0;
}
