#include "quantum_core.h"

void apply_gate_x(qubit_t *q){
  complex_q14_t temp = q->alpha;
  q->alpha = q->beta;
  q->beta  = temp;
}

static int16_t saturate_q14(int32_t val)
{
  if (val > 16383)  return 16383;
  if (val < -16384) return -16384;
  return (int16_t)val;
}

void apply_gate_h(qubit_t *q){
  complex_q14_t a = q->alpha;
  complex_q14_t b = q->beta;

  const int16_t s = Q14_INV_SQRT2;// 11585 = 0.7071

  int32_t sum_r = (int32_t)a.real + b.real;
  int32_t sum_i = (int32_t)a.imag + b.imag;

  //overflow check, this could be removed if needed
  q->alpha.real = saturate_q14(fp_mul(s, (int16_t)(sum_r >> 1)) +
                               fp_mul(s, (int16_t)(sum_r & 1 ? 1 : 0)));
  q->alpha.imag = saturate_q14(fp_mul(s, (int16_t)(sum_i >> 1)) +
                               fp_mul(s, (int16_t)(sum_i & 1 ? 1 : 0)));

  int32_t diff_r = (int32_t)a.real - b.real;
  int32_t diff_i = (int32_t)a.imag - b.imag;

  //overflow check, this could be removed if needed
  q->beta.real = saturate_q14(fp_mul(s, (int16_t)(diff_r >> 1)) +
                              fp_mul(s, (int16_t)(diff_r & 1 ? (diff_r > 0 ? 1 : -1) : 0)));
  q->beta.imag = saturate_q14(fp_mul(s, (int16_t)(diff_i >> 1)) +
                              fp_mul(s, (int16_t)(diff_i & 1 ? (diff_i > 0 ? 1 : -1) : 0)));
}
