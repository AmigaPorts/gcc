extern _Complex double __muldc3(double, double, double, double);
double to_double(long long x) { return x; }
long long to_longlong(double x) { return x; }
unsigned long long to_unsigned(double x) { return x; }
double power(double x, int n) { return __builtin_powi(x, n); }
_Complex double complex_mul(_Complex double a, _Complex double b)
{
  return __muldc3(__real__ a, __imag__ a, __real__ b, __imag__ b);
}
