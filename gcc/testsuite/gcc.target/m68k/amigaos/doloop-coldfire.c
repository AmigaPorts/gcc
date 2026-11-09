/* { dg-do compile } */
/* { dg-options "-O2 -mcpu=5475" } */

/* ColdFire has no dbra: a loop counter narrowed to 16 bits for it
   only costs word copies and tests.  */

volatile unsigned long cnt;

void
count (unsigned n)
{
  unsigned m = n & 0xfffe;
  for (unsigned i = 0; i < m; i++)
    cnt++;
}

/* { dg-final { scan-assembler-not {\.w } } } */
