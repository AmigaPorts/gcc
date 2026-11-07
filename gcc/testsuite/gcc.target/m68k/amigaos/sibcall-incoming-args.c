/* Amiga register-parameter ABI: a call that passes the caller's own
   register arguments on unchanged, in the same call-saved registers, can
   be a sibcall.  Those registers hold the incoming values when the
   epilogue jumps, whether it restores them or not.  A call that swaps
   them, or passes a new value, stays a jsr.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -O2 -fomit-frame-pointer" } */

struct BoardInfo;

extern void invert (struct BoardInfo *bi __asm ("a0"), short x __asm ("d0"),
		    short w __asm ("d2"), short h __asm ("d3"),
		    unsigned char mask __asm ("d4"),
		    unsigned long f __asm ("d7"));
extern long pair (long a __asm ("d2"), long b __asm ("d3"));
extern long pair_new (long a __asm ("d2"), long b __asm ("d3"));

void
forward (struct BoardInfo *bi __asm ("a0"), short x __asm ("d0"),
	 short w __asm ("d2"), short h __asm ("d3"),
	 unsigned char mask __asm ("d4"), unsigned long f __asm ("d7"))
{
  invert (bi, x, w, h, mask, f);
}

long
swapped (long a __asm ("d2"), long b __asm ("d3"))
{
  return pair (b, a);
}

long
changed (long a __asm ("d2"), long b __asm ("d3"))
{
  return pair_new (a + 1, b);
}

/* { dg-final { scan-assembler-times "jra _invert" 1 } } */
/* { dg-final { scan-assembler-not "jsr _invert" } } */
/* { dg-final { scan-assembler-times "jsr _pair\\M" 1 } } */
/* { dg-final { scan-assembler-times "jsr _pair_new" 1 } } */
