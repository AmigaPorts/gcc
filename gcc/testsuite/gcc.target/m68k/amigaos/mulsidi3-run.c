/* { dg-do run } */
/* { dg-options "-m68020-60" } */

/* Check the -m68020-60 widening multiplies against a reference built
   from 16-bit halves in 32-bit arithmetic only, on edge values and on
   pseudo-random operands. */

#include <stdlib.h>

typedef unsigned long u32;
typedef unsigned long long u64;
typedef long long s64;

__attribute__((noinline)) u64 umul (u32 a, u32 b) { return (u64) a * b; }
__attribute__((noinline)) s64 smul (long a, long b) { return (s64) a * b; }
__attribute__((noinline)) u32 umulhi (u32 a, u32 b)
{
  return ((u64) a * b) >> 32;
}

/* The unsigned product of A and B as HI:LO, with no 64-bit type. */
static void
ref_umul (u32 a, u32 b, u32 *hi, u32 *lo)
{
  u32 al = a & 0xffff, ah = a >> 16, bl = b & 0xffff, bh = b >> 16;
  u32 ll = al * bl, lh = al * bh, hl = ah * bl, hh = ah * bh;
  u32 t = lh + (ll >> 16);
  u32 u = hl + (t & 0xffff);

  *lo = (u << 16) | (ll & 0xffff);
  *hi = hh + (t >> 16) + (u >> 16);
}

static void
check (u32 a, u32 b)
{
  u32 hi, lo;
  u64 p;
  s64 s;

  ref_umul (a, b, &hi, &lo);
  p = umul (a, b);
  if ((u32) (p >> 32) != hi || (u32) p != lo || umulhi (a, b) != hi)
    abort ();
  /* The signed product differs from the unsigned one only in its high
     word: minus B if A is negative, minus A if B is. */
  if ((long) a < 0)
    hi -= b;
  if ((long) b < 0)
    hi -= a;
  s = smul ((long) a, (long) b);
  if ((u32) ((u64) s >> 32) != hi || (u32) s != lo)
    abort ();
}

static const u32 edge[] = {
  0, 1, 2, 0xffff, 0x10000, 0x7fff, 0x8000, 0xffffffff, 0x80000000,
  0x7fffffff, 0x80000001, 0xfffffffe, 0x0001ffff, 0xffff0000, 0x12345678,
};

int
main (void)
{
  u32 x = 1;
  unsigned i, j;

  for (i = 0; i < sizeof edge / sizeof edge[0]; i++)
    for (j = 0; j < sizeof edge / sizeof edge[0]; j++)
      check (edge[i], edge[j]);
  for (i = 0; i < 2000; i++)
    {
      u32 a, b;

      x = x * 1664525 + 1013904223;
      a = x;
      x = x * 1664525 + 1013904223;
      b = x >> (x & 31);
      check (a, b);
    }
  return 0;
}
