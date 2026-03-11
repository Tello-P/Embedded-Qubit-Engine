#include "quantum_core.h"


uint64_t rng_state = 0x853c49e6748fea9bULL;//Initial seed for random

// xorshift* 64-bit for random
static inline uint64_t rotl(const uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}
static uint64_t next_random_u64(void) {
  uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}
#define Q14_MAX 16384
int16_t random_q14(void) {
  uint64_t r = next_random_u64();
  
   
  uint32_t val = ((uint32_t)(r >> 48) * (uint32_t)Q14_MAX) >> 16;
  return (int16_t) val;
}

void apply_gate_x(qubit_t *q){
  complex_q14_t temp = q->alpha;
  q->alpha = q->beta;
  q->beta  = temp;
}


void apply_gate_h(qubit_t *q){
  int32_t ar = q->alpha.real;
  int32_t br = q->beta.real;
  int32_t ai = q->alpha.imag;
  int32_t bi = q->beta.imag;

  const int16_t s = Q14_INV_SQRT2;

  // a_new = (a + b) / sqrt(2)
  q->alpha.real = (int16_t)(((ar + br) * s + Q14_HALF) >> 14);
  q->alpha.imag = (int16_t)(((ai + bi) * s + Q14_HALF) >> 14);

  // b_new = (a - b) / sqrt(2)
  q->beta.real = (int16_t)(((ar - br) * s + Q14_HALF) >> 14);
  q->beta.imag = (int16_t)(((ai - bi) * s + Q14_HALF) >> 14);
}

void apply_gate_cnot(quantum_register_t *reg, uint8_t control, uint8_t target) {
  uint16_t control_mask = (1u << control);
  uint16_t target_mask = (1u << target);

  for (uint16_t i = 0; i < reg->dim; i++) {
    if ((i & control_mask) && !(i & target_mask)) {
      uint16_t i0 = i;
      uint16_t i1 = i | target_mask;

      complex_q14_t temp = reg->state[i0];
      reg->state[i0] = reg->state[i1];
      reg->state[i1] = temp;
    }
  }
}


int measure(qubit_t *q){
  int16_t alpha_sq = complex_mag_sq(q->alpha);
  int16_t random = random_q14();

  if(random<alpha_sq){
    q->alpha.real = Q14_ONE;
    q->alpha.imag = 0;
    q->beta.real  = 0;
    q->beta.imag  = 0;
    return 0;
  }else{
    q->alpha.real = 0;
    q->alpha.imag = 0;
    q->beta.real  = Q14_ONE;
    q->beta.imag  = 0;
    return 1;
  }
}
