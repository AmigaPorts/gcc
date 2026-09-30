/* The soft-float long double add returned the second operand when the
   first was zero, so +0 + -0 came out as -0 and, through __subxf3,
   so did +0 - +0.  IEEE 754 gives +0 for both in round-to-nearest; the
   sum of two zeros is -0 only when both are.  These are the cases
   c-c++-common/torture/complex-sign-*.c fail on.  */
/* { dg-do run } */
/* { dg-skip-if "long double is in hardware" { *-*-* } { "-m68881" "-mhard-float" } { "" } } */

volatile long double pz = 0.0L, nz = -0.0L, one = 1.0L;

int
main (void)
{
  if (__builtin_signbit (pz + nz))
    __builtin_abort ();
  if (__builtin_signbit (nz + pz))
    __builtin_abort ();
  if (!__builtin_signbit (nz + nz))
    __builtin_abort ();
  if (__builtin_signbit (pz - pz))
    __builtin_abort ();
  if (__builtin_signbit (nz - nz))
    __builtin_abort ();
  if (!__builtin_signbit (nz - pz))
    __builtin_abort ();
  if (__builtin_signbit (one - one))
    __builtin_abort ();
  return 0;
}
