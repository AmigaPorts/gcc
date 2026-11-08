/* Amiga register-parameter ABI: a double bound to d0 takes d0 and d1, and
   with -mregparm the arguments after it go in d2, d3 and a0.  A forwarder
   whose own d2 and d3 hold other values must load them, so f gets the
   arguments in their order.  */

/* { dg-do run } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-mregparm=4" } */

extern void abort (void);

__attribute__ ((noipa)) long
f (double x __asm ("d0"), long y, long z, long w)
{
  return (long) (x * 2) * 1000000 + y * 10000 + z * 100 + w;
}

__attribute__ ((noipa)) long
g (long z __asm ("d2"), long w __asm ("d3"), long y)
{
  return f (1.5, y, z, w);
}

int
main (void)
{
  if (g (34, 56, 12) != 3123456)
    abort ();
  return 0;
}
