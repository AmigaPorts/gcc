/* Amiga register-parameter ABI: forwarders that pass their arguments on
   in the same call-saved registers tail-jump, and the callee gets them
   whole.  Forwarders that swap or change them, and calls to the current
   function, pass the values the source names.  */

/* { dg-do run } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-fomit-frame-pointer -mregparm=4" } */

extern void abort (void);

typedef struct _Pair { long v; } Pair;

__attribute__ ((noipa)) long
sum (short w __asm ("d2"), long h __asm ("d3"),
     unsigned char mask __asm ("d4"), unsigned long f __asm ("d7"))
{
  return w * 1000000L + h * 10000 + mask * 100 + (long) f;
}

__attribute__ ((noipa)) long
forward (short w __asm ("d2"), long h __asm ("d3"),
	 unsigned char mask __asm ("d4"), unsigned long f __asm ("d7"))
{
  return sum (w, h, mask, f);
}

__attribute__ ((noipa)) long
swapped (short w __asm ("d2"), long h __asm ("d3"),
	 unsigned char mask __asm ("d4"), unsigned long f __asm ("d7"))
{
  return sum ((short) h, w, mask, f + 1);
}

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
  return (Pair *) (x * 100 + y * 10 + q);
}

__attribute__ ((noinline)) Pair *
diff (long p __asm ("d0"), long q __asm ("d1"),
      long x __asm ("d3"), long y __asm ("d2"), volatile long n __asm ("a0"))
{
  if (n)
    return diff (p, q, y, x, n - 1);
  return (Pair *) (x * 100 + y * 10 + q);
}

int
main (void)
{
  if (forward (-12, 34, 200, 56) != -12000000L + 340000 + 20000 + 56)
    abort ();
  if (swapped (-12, 34, 200, 56) != 34000000L - 120000 + 20000 + 57)
    abort ();
  if (same (0, 1, 2, 3, 3) != (Pair *) 231)
    abort ();
  if (diff (0, 1, 2, 3, 3) != (Pair *) 321)
    abort ();
  if (diff (0, 1, 2, 3, 2) != (Pair *) 231)
    abort ();
  return 0;
}
