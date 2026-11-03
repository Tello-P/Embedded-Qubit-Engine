/* qsim_run: ejecuta un circuito pasado por argv sobre el motor.
 *   ./qsim_run N [--seed S] h 0 cx 0 1 x 2 z 1 m 0
 * stdout: estado crudo, una linea "real imag" por amplitud (Q1.14, int16).
 * stderr: una linea "M qubit resultado" por medida.
 * Puertas: h, x, z (1 qubit), cx (control target), m (medida),
 *          o K (apply_oracle sobre el indice K), d (apply_diffuser). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quantum_core.h"
#include "quantum_register.h"

static int die(const char *msg, const char *arg) {
    fprintf(stderr, "qsim_run: %s%s%s\n", msg, arg ? ": " : "", arg ? arg : "");
    return 2;
}

static int parse_u8(const char *s, unsigned max, uint8_t *out) {
    char *end;
    long v = strtol(s, &end, 10);
    if (*s == '\0' || *end != '\0' || v < 0 || v >= (long)max) return -1;
    *out = (uint8_t)v;
    return 0;
}

int main(int argc, char **argv) {
    int i = 1;
    uint8_t n;
    if (argc < 2 || parse_u8(argv[i++], 255, &n)) return die("uso: qsim_run N [--seed S] circuito", NULL);
    if (i + 1 < argc && strcmp(argv[i], "--seed") == 0) {
        seed_quantum_rng(strtoull(argv[i + 1], NULL, 10));
        i += 2;
    }
    quantum_register_t reg = init_register(n);
    if (reg.dim == 0) return die("registro invalido (n fuera de rango)", argv[1]);

    while (i < argc) {
        const char *op = argv[i++];
        uint8_t a, b;
        if (!strcmp(op, "cx")) {
            if (i + 1 >= argc || parse_u8(argv[i], n, &a) || parse_u8(argv[i + 1], n, &b) || a == b)
                return die("cx: argumentos invalidos", NULL);
            apply_gate_cnot(&reg, a, b);
            i += 2;
        } else if (!strcmp(op, "d")) {
            apply_diffuser(&reg);
        } else if (!strcmp(op, "o")) {
            if (i >= argc || parse_u8(argv[i], reg.dim, &a)) return die("o: indice invalido", NULL);
            apply_oracle(&reg, a);
            i++;
        } else if (!strcmp(op, "h") || !strcmp(op, "x") || !strcmp(op, "z") || !strcmp(op, "m")) {
            if (i >= argc || parse_u8(argv[i], n, &a)) return die("qubit invalido", op);
            i++;
            if (op[0] == 'h') apply_gate_h(&reg, a);
            else if (op[0] == 'x') apply_gate_x(&reg, a);
            else if (op[0] == 'z') apply_gate_z(&reg, a);
            else fprintf(stderr, "M %u %d\n", a, measure_qubit(&reg, a));
        } else {
            return die("puerta desconocida", op);
        }
    }
    for (uint16_t k = 0; k < reg.dim; k++) printf("%d %d\n", reg.state[k].real, reg.state[k].imag);
    return 0;
}
