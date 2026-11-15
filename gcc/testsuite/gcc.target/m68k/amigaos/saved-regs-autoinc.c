/* Amiga register-parameter ABI: a call-saved register that holds an
   argument is written when the function walks it with (An)+, though no
   insn sets it, so it is saved.  A register that is only read is not.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -fomit-frame-pointer" } */

void
clear (unsigned char *p __asm ("a2"), short n __asm ("d0"))
{
  while (n--)
    *p++ = 0;
}

long
read_only (long a __asm ("d2"))
{
  return a + 1;
}

/* { dg-final { scan-assembler "clr.b \\(a2\\)\\+" } } */
/* { dg-final { scan-assembler-times "a2,-\\(sp\\)" 1 } } */
/* { dg-final { scan-assembler-not "d2,-\\(sp\\)" } } */
