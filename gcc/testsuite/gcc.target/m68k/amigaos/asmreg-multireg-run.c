/* Amiga register-parameter ABI: a _Complex int bound to d0 takes d0 and
   d1, so with -mregparm the arguments after it go in d2 and a0, and the
   imaginary part in d1 stays intact.  */

/* { dg-do run } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-mregparm=3" } */

extern void abort (void);

__attribute__ ((noipa)) long
f (_Complex int c __asm ("d0"), long x, long y)
{
  return __real__ c * 1000000 + __imag__ c * 10000 + x * 100 + y;
}

__attribute__ ((noipa)) long
g (long b __asm ("d2"))
{
  return f (12 + 34i, 56, b);
}

int
main (void)
{
  if (g (78) != 12345678)
    abort ();
  return 0;
}
