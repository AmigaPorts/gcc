/* { dg-do compile } */
/* { dg-options "-O2" } */
/* A saveallregs function saves every register but sp, minus those that
   hold its return value: d0-d7/a0-a6 for void, d1-d7/a0-a6 for int, and
   d2-d7/a0-a6 for long long.  */

extern int g;

__saveallregs void fv (void) { g++; }
__saveallregs int fi (void) { return g + 1; }
__saveallregs long long fll (void) { return (long long) g << 32; }

/* { dg-final { scan-assembler-times {movem\.l #65534,-\(sp\)} 1 } } */
/* { dg-final { scan-assembler-times {movem\.l #32766,-\(sp\)} 1 } } */
/* { dg-final { scan-assembler-times {movem\.l #16382,-\(sp\)} 1 } } */
