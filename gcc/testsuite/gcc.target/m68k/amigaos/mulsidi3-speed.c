/* { dg-do compile } */
/* { dg-options "-O2 -m68020-60" } */

/* The 68060 has no MULU.L/MULS.L with a 64-bit result, so -m68020-60
   expands a widening multiply with four 16-bit MULU.W instead of
   calling __muldi3. */

typedef unsigned long long u64;
typedef long long s64;

u64 umul (unsigned long a, unsigned long b) { return (u64) a * b; }
s64 smul (long a, long b) { return (s64) a * b; }
u64 mul64 (u64 a, u64 b) { return a * b; }

/* { dg-final { scan-assembler-not "__muldi3" } } */
/* { dg-final { scan-assembler-not "mul\[su\]\\.?l\[^\\n\]*:" } } */
/* { dg-final { scan-assembler-times "mulu\\.?w" 12 } } */
