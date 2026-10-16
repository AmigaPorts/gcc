/* The soft-float long double code returned an infinity for Inf - Inf,
   Inf * 0, Inf / Inf and 0 / 0: the "NaN" it built had only the integer
   bit set.  Its infinities had that bit set too, unlike the ones GCC
   emits for constants, and the compares told the two apart.  */
/* { dg-do run } */
/* { dg-skip-if "long double is in hardware" { *-*-* } { "-m68881" "-mhard-float" } { "" } } */

volatile long double big = 1e4000L, inf = __builtin_infl (), zero = 0.0L,
  two = 2.0L;

/* Infinities with the integer bit set, as older libgcc built them.  */
static const unsigned pinf_j[3] = { 0x7fff0000, 0x80000000, 0 };
static const unsigned ninf_j[3] = { 0xffff0000, 0x80000000, 0 };

int
main (void)
{
  long double ov = big * big;
  volatile long double pj, nj, ninf = -inf;

  __builtin_memcpy ((void *) &pj, pinf_j, 12);
  __builtin_memcpy ((void *) &nj, ninf_j, 12);
  if (pj != inf || inf != pj || nj != ninf || ninf != nj)
    __builtin_abort ();
  if (pj < inf || pj > inf || !(pj <= inf) || !(inf >= pj))
    __builtin_abort ();
  if (nj < ninf || ninf > nj || !(nj < pj) || !(pj > big) || !(nj < -big))
    __builtin_abort ();

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
