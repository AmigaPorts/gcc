/* { dg-do compile } */
/* { dg-options "-O2 -m68020-60" } */

/* The 68060 has no MULU.L/MULS.L with a 64-bit result, so -m68020-60
   expands a widening multiply with four 16-bit MULU.W instead of
   calling __muldi3.  A division by a constant still uses DIVU.L
   rather than a multiply by the reciprocal through that sequence. */

typedef unsigned long long u64;
typedef long long s64;

u64 umul (unsigned long a, unsigned long b) { return (u64) a * b; }
s64 smul (long a, long b) { return (s64) a * b; }
u64 mul64 (u64 a, u64 b) { return a * b; }
unsigned long udiv10 (unsigned long a) { return a / 10; }

/* { dg-final { scan-assembler-not "__muldi3" } } */
/* No 64-bit MULU.L/MULS.L: its register pair has a colon (d1:d0); the
   32-bit form of mul64's cross terms is fine. */
/* { dg-final { scan-assembler-not "mul\[su\]\\.?l\[^\\n\]*:" } } */
/* Four for each of the three widening multiplies, none for udiv10. */
/* { dg-final { scan-assembler-times "mulu\\.?w" 12 } } */
/* { dg-final { scan-assembler-times "divu\\.?l" 1 } } */
