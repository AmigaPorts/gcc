/* -mschulz saves and restores the call-saved FP registers with one fmove.d
   each instead of fmovem.

   - f3 keeps three doubles live across calls, in fp2-fp4.
   - They are pushed fp4 first and popped fp2 first, as fmovem would.
   - The simulator board has no FPU, so this is a compile test.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68040 -m68881 -fomit-frame-pointer -mschulz" } */
/* { dg-final { scan-assembler-not "fmovem" } } */
/* { dg-final { scan-assembler {fmove\.d fp4,-\(sp\)\n\tfmove\.d fp3,-\(sp\)\n\tfmove\.d fp2,-\(sp\)\n} } } */
/* { dg-final { scan-assembler {move\.d \(sp\)\+,fp2\n\tf[ds]?move\.d \(sp\)\+,fp3\n\tf[ds]?move\.d \(sp\)\+,fp4\n} } } */

double g (double);

double
f3 (double a, double b, double c)
{
  double x = g (a), y = g (b);
  return x * a + y * b + g (c) * c;
}
