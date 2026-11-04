extern double to_double(long long);
extern long long to_longlong(double);
extern unsigned long long to_unsigned(double);
extern double power(double, int);
extern _Complex double complex_mul(_Complex double, _Complex double);
int test_main(void)
{
  if (to_double(0x123456789LL) != 4886718345.0) return 1;
  if (to_longlong(-4886718345.75) != -4886718345LL) return 2;
  if (to_unsigned(4294967296.0) != 4294967296ULL) return 3;
  if (power(1.5, 3) != 3.375) return 4;
  _Complex double a = 1.0 + 2.0i, b = 3.0 + 4.0i;
  _Complex double c = complex_mul(a, b);
  if (__real__ c != -5.0 || __imag__ c != 10.0) return 5;
  return 0;
}
