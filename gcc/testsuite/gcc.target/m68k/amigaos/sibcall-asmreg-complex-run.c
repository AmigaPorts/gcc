/* Amiga register-parameter ABI: a _Complex int bound to d1 takes d1 and
   d2, and a forwarder that loads it there passes the imaginary part in
   d2, not its own saved d2.  */

/* { dg-do run } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-fomit-frame-pointer" } */

extern void abort (void);

__attribute__ ((noipa)) long
f (_Complex int c __asm ("d1"), long b __asm ("d3"))
{
  return __real__ c * 10000 + __imag__ c * 100 + b;
}

__attribute__ ((noipa)) long
g (_Complex int *p __asm ("a0"), long b __asm ("d3"))
{
  return f (*p, b);
}

__attribute__ ((noipa)) long
call_g (_Complex int *p, long b, long d2 __asm ("d2"))
{
  return g (p, b) + d2;
}

int
main (void)
{
  _Complex int c = 12 + 34i;
  if (call_g (&c, 56, 7000000) != 7123456)
    abort ();
  return 0;
}
