#include "fixed_point.h"

int16_t fp_mul(int16_t a, int16_t b) {
    int32_t result = (int32_t)a * (int32_t)b;
    return (int16_t)((result + 8192) >> 14);
}

complex_q14_t complex_add(complex_q14_t a, complex_q14_t b) {
    return (complex_q14_t){a.real + b.real, a.imag + b.imag};
}

complex_q14_t complex_mul(complex_q14_t a, complex_q14_t b) {
    complex_q14_t res;
    res.real = fp_mul(a.real, b.real) - fp_mul(a.imag, b.imag);
    res.imag = fp_mul(a.real, b.imag) + fp_mul(a.imag, b.real);
    return res;
}
