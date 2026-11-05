#include "bench.h"

/* Compile separately at -O0, without LTO or fast-math.  Every output is
   compared with this scalar reference outside the measured interval.  */
#define MATRIX(NAME, TYPE) \
void NAME (TYPE *out, const TYPE *m, const TYPE *v) \
{ \
  for (unsigned r = 0; r < 4; ++r) \
    { \
      TYPE sum = m[r*4] * v[0]; \
      for (unsigned c = 1; c < 4; ++c) \
	sum = sum + m[r*4+c] * v[c]; \
      out[r] = sum; \
    } \
}
MATRIX (reference_matrix_f, float)
MATRIX (reference_matrix_d, double)

#define MANDEL(NAME, TYPE) \
unsigned NAME (TYPE cr, TYPE ci) \
{ \
  TYPE real = 0, imag = 0; \
  unsigned n = 0; \
  for (;;) \
    { \
      TYPE real2 = real * real, imag2 = imag * imag; \
      if (n == ITERATION_LIMIT || real2 + imag2 >= (TYPE) 4) return n; \
      TYPE next_imag = (TYPE) 2 * real * imag + ci; \
      real = real2 - imag2 + cr; \
      imag = next_imag; \
      ++n; \
    } \
}
MANDEL (reference_mandel_f, float)
MANDEL (reference_mandel_d, double)
