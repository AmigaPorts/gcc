/* Amiga register-parameter ABI: only arguments in call-clobbered registers
   (d0, d1, a0, a1) allow a sibcall.  An argument in a call-saved register
   keeps the call a jsr: the sibcall epilogue would restore that register
   from the caller's save area after the argument was loaded.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -O2 -fomit-frame-pointer" } */

extern long saved_args (long a __asm ("d2"), long b __asm ("a2"));
extern long scratch_args (long a __asm ("d0"), long b __asm ("a1"));

long
call_saved (long x, long y)
{
  return saved_args (x, y);
}

long
call_scratch (long x, long y)
{
  return scratch_args (x, y);
}

/* { dg-final { scan-assembler-times "jsr _saved_args" 1 } } */
/* { dg-final { scan-assembler-not "jra _saved_args" } } */
/* { dg-final { scan-assembler-times "jra _scratch_args" 1 } } */
