/* Amiga register-parameter ABI: a double bound to d0 takes d0 and d1, so
   with -mregparm the arguments after it go in d2, d3 and a0.  Here those
   are not the caller's incoming d2 and d3, so the call is no sibcall.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -O2 -fomit-frame-pointer -mregparm=4" } */

extern long f (double x __asm ("d0"), long y, long z, long w);

long
g (long z __asm ("d2"), long w __asm ("d3"), long y)
{
  return f (1.5, y, z, w);
}

/* { dg-final { scan-assembler "jsr _f" } } */
/* { dg-final { scan-assembler-not "jra _f" } } */
