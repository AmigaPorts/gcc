/* Compile this translation unit with the conventional PowerPC ABI.  */
extern void spe_dot_product(const double *, const double *, unsigned, double *);
static const double a[] = { 1.5, 2.0, -1.0 };
static const double b[] = { 2.0, 3.0, 0.5 };
int test_main(void)
{
  union { double value; unsigned long long bits; } result;
  spe_dot_product(a, b, 3, &result.value);
  return result.bits != 0x4021000000000000ULL; /* 8.5 */
}
