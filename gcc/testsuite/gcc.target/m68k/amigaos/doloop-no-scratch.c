/* { dg-do compile } */
/* { dg-options "-O2 -fomit-frame-pointer -m68040" } */

/* The dbra loop needs no register besides its counter: nothing may be
   allocated to a callee-saved register, and saved, for it. */
void
fill (short *d, short n)
{
  while (n--)
    *d++ = n;
}

/* { dg-final { scan-assembler "dbra" } } */
/* { dg-final { scan-assembler-not "d2" } } */
