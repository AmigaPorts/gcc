/* { dg-do compile } */
/* { dg-options "-O2 -m68040 -m68881 -fexcess-precision=standard" } */
/* libgcc's ___floatdidf and ___floatundidf return the extended value, so
   a long long converts to double through their _rounded versions.  int to
   float needs nothing, fsmove rounds; but an int operand of fsadd and the
   others would be added exactly, without its own rounding to float.  */

double
di_df (long long i)
{
  return (double) i;
}

double
udi_df (unsigned long long i)
{
  return (double) i;
}

float
si_sf (int i)
{
  return (float) i;
}

float
add_if (float a, int i)
{
  return a + (float) i;
}

float
mul_if (float a, int i)
{
  return a * (float) i;
}

/* { dg-final { scan-assembler {___floatdidf_rounded} } } */
/* { dg-final { scan-assembler {___floatundidf_rounded} } } */
/* { dg-final { scan-assembler-not {fs(add|mul)\.l} } } */
/* { dg-final { scan-assembler-times {fsmove\.l} 3 } } */
