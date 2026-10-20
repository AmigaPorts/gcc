/* { dg-do compile } */
/* { dg-options "-Os -m68020-60" } */

/* At -Os the 16-bit expansion of mulsidi3-speed.c is not used: each
   multiply is one __muldi3 call, as without the pattern, and a 64x64
   multiply does not compute its cross terms inline before the call. */

typedef unsigned long long u64;
typedef long long s64;

u64 umul (unsigned long a, unsigned long b) { return (u64) a * b; }
s64 smul (long a, long b) { return (s64) a * b; }
u64 mul64 (u64 a, u64 b) { return a * b; }

/* { dg-final { scan-assembler-times "__muldi3" 3 } } */
/* { dg-final { scan-assembler-not "mul\[su\]\\.?\[wl\]" } } */
