/* Amiga register-parameter ABI: a read-only argument in a call-saved
   register is dead after its last read in a block that ends in a noreturn
   call, so a late pass may reuse the register there.  When that call can
   throw, the unwinder needs the caller's value, so the register is saved.
   A noreturn call that cannot throw keeps it unsaved.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -O2 -fomit-frame-pointer -fexceptions" } */

extern void fail (void) __attribute__ ((noreturn));
extern void quit (void) __attribute__ ((noreturn, nothrow));

long
may_throw (long a __asm ("d2"), long b)
{
  if (b)
    fail ();
  return a + b;
}

long
no_throw (long a __asm ("d3"), long b)
{
  if (b)
    quit ();
  return a + b;
}

/* { dg-final { scan-assembler-times "d2,-\\(sp\\)" 1 } } */
/* { dg-final { scan-assembler-not "d3,-\\(sp\\)" } } */
/* { dg-final { scan-assembler-not "movem" } } */
