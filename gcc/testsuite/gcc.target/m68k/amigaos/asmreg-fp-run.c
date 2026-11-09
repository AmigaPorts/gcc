/* Amiga register-parameter ABI: float arguments of every mode bound to FP
   registers, call-clobbered and call-saved ones, next to a data register
   argument, reach the callee in the registers it reads them from.  */

/* { dg-do run } */
/* { dg-require-effective-target m68881_runtime } */
/* { dg-options "-m68020 -m68881" } */

extern void abort (void);

__attribute__ ((noipa)) double
add_df (double a __asm ("fp0"), double b __asm ("fp1"))
{
  return a + b;
}

__attribute__ ((noipa)) float
mul_sf (float a __asm ("fp1"), float b __asm ("fp0"))
{
  return a * b;
}

__attribute__ ((noipa)) long double
sub_xf (long double a __asm ("fp2"), long double b __asm ("fp7"))
{
  return a - b;
}

__attribute__ ((noipa)) double
mix (long n __asm ("d0"), double x __asm ("fp3"), float y __asm ("fp0"))
{
  return n * x + y;
}

int
main (void)
{
  if (add_df (1.5, 2.25) != 3.75)
    abort ();
  if (mul_sf (3.0f, -0.5f) != -1.5f)
    abort ();
  if (sub_xf (10.0L, 0.25L) != 9.75L)
    abort ();
  if (mix (3, 0.5, 0.25f) != 1.75)
    abort ();
  return 0;
}
