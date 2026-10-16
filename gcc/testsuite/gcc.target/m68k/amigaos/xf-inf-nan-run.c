/* The soft-float long double code returned an infinity for Inf - Inf,
   Inf * 0, Inf / Inf and 0 / 0: the "NaN" it built had only the integer
   bit set.  Its infinities had that bit set too, unlike the ones GCC
   emits for constants, and the compares told the two apart.  */
/* { dg-do run } */
/* { dg-skip-if "long double is in hardware" { *-*-* } { "-m68881" "-mhard-float" } { "" } } */

volatile long double big = 1e4000L, inf = __builtin_infl (), zero = 0.0L,
  two = 2.0L;

int
main (void)
{
  long double ov = big * big;

  if (ov != inf || !(ov > big) || !__builtin_isinf (ov))
    __builtin_abort ();
  if (two / zero != inf || inf * two != inf)
    __builtin_abort ();
  if (!__builtin_isnan (inf - inf) || !__builtin_isnan (inf * zero)
      || !__builtin_isnan (zero * inf) || !__builtin_isnan (inf / inf)
      || !__builtin_isnan (zero / zero))
    __builtin_abort ();
  return 0;
}
