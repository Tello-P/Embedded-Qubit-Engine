#ifndef QUANTUM_CORE_H
#define QUANTUM_CORE_H

#include <stdint.h>
#include "fixed_point.h"

typedef struct {
     complex_q14_t alpha;
     complex_q14_t beta;
} qubit_t;

#endif

