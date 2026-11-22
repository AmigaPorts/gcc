/* { dg-do compile } */
/* { dg-options "-O2 -m68020-60 -m68881 -fexcess-precision=fast" } */
/* With -fexcess-precision=fast, the default for GNU C, the conversions keep
   the excess precision and cost nothing more: a bare fmove.l, and libgcc
   called directly.  */

float
si_sf (int i)
{
  return (float) i;
}

double
di_df (long long i)
{
  return (double) i;
}

/* { dg-final { scan-assembler-times {fmove\.l} 1 } } */
/* { dg-final { scan-assembler {___floatdidf} } } */
/* { dg-final { scan-assembler-not {_rounded} } } */
/* { dg-final { scan-assembler-not {___floatdixf} } } */
