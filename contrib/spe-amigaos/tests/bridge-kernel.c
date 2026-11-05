/* Compile this translation unit with -mcpu=8548.  */
void spe_dot_product(const double *a, const double *b, unsigned count,
		     double *result)
{
  double sum = 0;
  for (unsigned i = 0; i < count; ++i)
    sum += a[i] * b[i];
  *result = sum;
}
