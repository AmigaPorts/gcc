/* Amiga register-parameter ABI: a call-saved register that holds an
   argument the function only reads still has the caller's value on
   return, so it needs no save.  A call-saved register the function writes,
   here through an asm clobber, is saved.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -O2 -fomit-frame-pointer" } */

long
read_only (long a __asm ("d2"), long b __asm ("a2"))
{
  return a + b;
}

long
written (long a __asm ("d3"))
{
  __asm__ volatile ("" : : : "d3");
  return a + 1;
}

/* { dg-final { scan-assembler-not "d2,-\\(sp\\)" } } */
/* { dg-final { scan-assembler-not "a2,-\\(sp\\)" } } */
/* { dg-final { scan-assembler-times "d3,-\\(sp\\)" 1 } } */
