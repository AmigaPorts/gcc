/* Amiga register-parameter ABI: the same __asm("reg") dropping is triggered by
   a `saveds` attribute on only one of the two declarations (a declaration
   mismatch that also forces the C front-end to composite the function type).
   The callee must still read its arguments from d0/d1, not the stack.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* saveds is only meaningful with -fbaserel; without it the attribute warns.  */
/* { dg-options "-O1 -fbaserel" } */

long f (long a __asm ("d0"), long b __asm ("d1"));		/* no saveds */

__attribute__((saveds))
long f (long a __asm ("d0"), long b __asm ("d1"))		/* saveds -> mismatch */
{
  return a + b;
}

/* xfail marks the pre-fix state; the fix commit drops it.  */
/* { dg-final { scan-assembler "add.l d1,d0" { xfail m68k-*-amigaos* } } } */
