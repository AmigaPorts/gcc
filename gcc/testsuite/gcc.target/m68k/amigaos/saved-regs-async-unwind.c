/* Amiga register-parameter ABI: with asynchronous unwind tables an
   unwind can start at any insn, so a read-only argument in a call-saved
   register is saved in a function with a dead end even when nothing in
   it can throw.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68040 -Os -fomit-frame-pointer -fno-exceptions" } */
/* { dg-additional-options "-fasynchronous-unwind-tables" } */

long
check (unsigned long x __asm ("d2"), long y)
{
  if (y)
    {
      if (x > 255)
	__builtin_trap ();
      __builtin_trap ();
    }
  return x + y;
}

/* { dg-final { scan-assembler-times "d2,-\\(sp\\)" 1 } } */
