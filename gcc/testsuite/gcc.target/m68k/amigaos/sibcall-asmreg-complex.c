/* Amiga register-parameter ABI: a _Complex int bound to d1 takes d1 and
   d2.  The sibcall epilogue would restore d2 over its imaginary part, so
   the call is no sibcall.  In d0 and d1 it is.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68020 -fomit-frame-pointer" } */

extern long f (_Complex int c __asm ("d1"), long b __asm ("d3"));
extern long g (_Complex int c __asm ("d1"));
extern long h (_Complex int c __asm ("d0"));

long
call_f (_Complex int *p __asm ("a0"), long b __asm ("d3"))
{
  return f (*p, b);
}

long
call_g (_Complex int *p __asm ("a0"))
{
  return g (*p);
}

long
call_h (_Complex int *p __asm ("a0"))
{
  return h (*p);
}

/* { dg-final { scan-assembler "jsr _f" } } */
/* { dg-final { scan-assembler-not "jra _f" } } */
/* { dg-final { scan-assembler "jsr _g" } } */
/* { dg-final { scan-assembler-not "jra _g" } } */
/* { dg-final { scan-assembler "jra _h" } } */
