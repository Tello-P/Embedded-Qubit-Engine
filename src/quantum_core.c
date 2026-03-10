#include "quantum_core.h"

void apply_gate_x(qubit_t *q){
  complex_q14_t temp = q->alpha;
  q->alpha = q->beta;
  q->beta  = temp;
}
