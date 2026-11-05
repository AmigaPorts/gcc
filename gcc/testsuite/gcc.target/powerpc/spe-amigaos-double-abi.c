/* { dg-do compile { target powerpc*-*-amigaos* } } */
/* { dg-options "-O2 -mcpu=8548" } */
/* { dg-final { scan-assembler "\\tefddiv " } } */
/* { dg-final { scan-assembler-not "\\t(stw|lwz)" } } */

double divide (double a, double b) { return a / b; }
double add (double a, double b) { return a + b; }

double
from_bits (unsigned long long bits)
{
  union { double value; unsigned long long bits; } u;
  u.bits = bits;
  return u.value;
}
