/* { dg-do compile } */
/* { dg-options "-O2 -m68020-60 -m68881" } */
/* A long double temporary goes in fp0, not in fp2, which the prologue
   would have to save: its class is FP_REGS, not ALL_REGS.  */

extern long double q;

void
k (int i)
{
  long double t = i;

  q = t * t + t;
}

/* { dg-final { scan-assembler-not {fmovem} } } */
/* { dg-final { scan-assembler-not {fp2} } } */
