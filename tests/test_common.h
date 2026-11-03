#ifndef TEST_COMMON_H
#define TEST_COMMON_H
/* Utilidades de test: cualquier fallo termina el programa con exit(1). */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "quantum_register.h"

#define NORM_TOL 2e-3  /* medido <= 7e-4 incluso con 80 puertas; ver docs/tests.md */

#define CHECK(cond, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n  ", __FILE__, __LINE__, #cond); \
        fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); exit(1); \
    } } while (0)

static inline double reg_norm(const quantum_register_t *reg) {
    double s = 0;
    for (uint16_t i = 0; i < reg->dim; i++) {
        double r = reg->state[i].real / 16384.0, im = reg->state[i].imag / 16384.0;
        s += r * r + im * im;
    }
    return sqrt(s);
}

static inline double reg_prob(const quantum_register_t *reg, uint16_t i) {
    double r = reg->state[i].real / 16384.0, im = reg->state[i].imag / 16384.0;
    return r * r + im * im;
}

/* La norma del estado debe seguir siendo 1 despues de cada circuito. */
#define assert_norm(reg) do { \
    double n_ = reg_norm(reg); \
    CHECK(fabs(n_ - 1.0) < NORM_TOL, "norma = %.6f (esperada 1)", n_); \
} while (0)

#endif
