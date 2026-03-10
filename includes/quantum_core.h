#ifndef QUANTUM_CORE_H
#define QUANTUM_CORE_H

#include "fixed_point.h"

typedef struct {
     complex_q14_t alpha;
     complex_q14_t beta;
} qubit_t;


void apply_gate_x(qubit_t *q);
void apply_gate_h(qubit_t *q);

int measure(qubit_t *q);
int16_t random_q14(void);

#endif

