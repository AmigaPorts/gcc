/* Amiga register-parameter ABI: a _Complex int bound to d0 takes d0 and
   d1, so with -mregparm the arguments after it go in d2 and a0.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68020 -fomit-frame-pointer -mregparm=3" } */

extern long f (_Complex int c __asm ("d0"), long x, long y);

long
g (long b __asm ("d2"))
{
  return f (1 + 2i, 999, b);
}

/* { dg-final { scan-assembler "move.l #999,d2" } } */
/* { dg-final { scan-assembler "move.l d2,a0" } } */
