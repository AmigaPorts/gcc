/* Amiga register-parameter ABI: a tail call to a function that takes its
   arguments in call-saved registers (d2, d3, a2 here) must stay a normal
   call.  The caller saves those registers for itself, and a sibcall
   epilogue would restore them after the arguments were loaded, so the
   callee would read the caller's caller's values.  The clobbers force the
   caller to save them.  Result-based, so valid at every -O level.  */

/* { dg-do run } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */

extern void abort (void);

__attribute__ ((noipa)) long
take3 (long a __asm ("d2"), long b __asm ("d3"), long c __asm ("a2"))
{
  return a * 10000 + b * 100 + c;
}

__attribute__ ((noipa)) long
caller (long x, long y, long z)
{
  __asm__ volatile ("" : : : "d2", "d3", "a2");
  return take3 (x + 1, y + 1, z + 1);
}

int
main (void)
{
  long a = 1, b = 2, c = 3;

  /* Live across the call in the registers caller() saves.  */
  __asm__ volatile ("" : "+d" (a), "+d" (b), "+a" (c));
  if (caller (11, 33, 55) != 123456)
    abort ();
  __asm__ volatile ("" : "+d" (a), "+d" (b), "+a" (c));
  if (a != 1 || b != 2 || c != 3)
    abort ();
  return 0;
}
