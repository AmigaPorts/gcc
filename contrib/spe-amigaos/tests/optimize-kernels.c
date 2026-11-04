typedef float v2sf __attribute__((vector_size(8)));

v2sf vector_add (v2sf a, v2sf b) { return a + b; }
v2sf vector_sub (v2sf a, v2sf b) { return a - b; }
v2sf vector_mul (v2sf a, v2sf b) { return a * b; }
v2sf vector_div (v2sf a, v2sf b) { return a / b; }
v2sf vector_neg (v2sf a) { return -a; }

void
array_add (float *restrict out, const float *restrict a,
	   const float *restrict b, unsigned n)
{
  for (unsigned i = 0; i < n; ++i)
    out[i] = a[i] + b[i];
}

void
array_add_int (int *restrict out, const int *restrict a,
	       const int *restrict b, unsigned n)
{
  for (unsigned i = 0; i < n; ++i)
    out[i] = a[i] + b[i];
}

void
array_abs (float *restrict out, const float *restrict a, unsigned n)
{
  for (unsigned i = 0; i < n; ++i)
    out[i] = __builtin_fabsf (a[i]);
}

double double_div (double a, double b) { return a / b; }
double double_add (double a, double b) { return a + b; }

/* Bitcasts must not require incompatible DF/SI register views in LRA.  */
unsigned long long
double_bits (double value)
{
  union { double value; unsigned long long bits; } u;
  u.value = value;
  return u.bits;
}
