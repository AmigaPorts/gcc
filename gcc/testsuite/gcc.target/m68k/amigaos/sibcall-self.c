/* Amiga register-parameter ABI: a call to the current function that
   passes its register arguments on unchanged is a sibcall like any other,
   and one that swaps them stays a jsr.  The prototype names the struct
   tag where the definition uses the typedef, and the volatile count keeps
   tail recursion from turning the calls into loops.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -O2 -fomit-frame-pointer -mregparm=4" } */

typedef struct _Pair { long v; } Pair;

struct _Pair *same (long p __asm ("d0"), long q __asm ("d1"),
		    long x __asm ("d3"), long y __asm ("d2"),
		    volatile long n __asm ("a0"));
struct _Pair *diff (long p __asm ("d0"), long q __asm ("d1"),
		    long x __asm ("d3"), long y __asm ("d2"),
		    volatile long n __asm ("a0"));

__attribute__ ((noinline)) Pair *
same (long p __asm ("d0"), long q __asm ("d1"),
      long x __asm ("d3"), long y __asm ("d2"), volatile long n __asm ("a0"))
{
  if (n)
    return same (p, q, x, y, n - 1);
  return (Pair *) (x - y + q);
}

__attribute__ ((noinline)) Pair *
diff (long p __asm ("d0"), long q __asm ("d1"),
      long x __asm ("d3"), long y __asm ("d2"), volatile long n __asm ("a0"))
{
  if (n)
    return diff (p, q, y, x, n - 1);
  return (Pair *) (x - y + q);
}

/* { dg-final { scan-assembler-times "jra _same" 1 } } */
/* { dg-final { scan-assembler-not "jsr _same" } } */
/* { dg-final { scan-assembler-times "jsr _diff" 1 } } */
/* { dg-final { scan-assembler-not "jra _diff" } } */
