/* Amiga register-parameter ABI: with unwind tables and no -fexceptions,
   an exception can still pass through a call in a block that ends in a
   noreturn call.  A read-only argument in a call-saved register that is
   dead there is saved, so the unwinder hands the caller its value.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68040 -O2 -fomit-frame-pointer -funwind-tables" } */

extern void die1 (void) __attribute__ ((noreturn));
extern void die2 (void) __attribute__ ((noreturn));

long
check (unsigned long x __asm ("d2"), long y)
{
  if (y)
    {
      if (x > 255)
	die1 ();
      die2 ();
    }
  return x + y;
}

/* { dg-final { scan-assembler-times "d2,-\\(sp\\)" 1 } } */
