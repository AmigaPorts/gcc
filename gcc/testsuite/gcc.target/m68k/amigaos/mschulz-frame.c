/* -mschulz with a frame pointer restores each FP register from its own
   8-byte slot below the frame pointer.

   - fp2 is at the lowest address; f3 keeps fp2-fp5 here.
   - With a frame of 32K or more (big), the epilogue addresses the slots
     through a1.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68040 -m68881 -fno-omit-frame-pointer -mschulz" } */
/* { dg-final { scan-assembler-not "fmovem" } } */
/* { dg-final { scan-assembler {move\.d -32\(a5\),fp2\n\tf[ds]?move\.d -24\(a5\),fp3\n\tf[ds]?move\.d -16\(a5\),fp4\n\tf[ds]?move\.d -8\(a5\),fp5\n} } } */
/* { dg-final { scan-assembler {move\.d -16\(a1,a5\.l\),fp2\n\tf[ds]?move\.d -8\(a1,a5\.l\),fp3\n} } } */

double g (double);

double
f3 (double a, double b, double c)
{
  double x = g (a), y = g (b);
  return x * a + y * b + g (c) * c;
}

double
big (double a, double b)
{
  volatile char buf[40000];
  double x = g (a);
  buf[0] = 1;
  return x * a + g (b) * b + buf[0];
}
