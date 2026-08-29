/* Amiga register-parameter ABI: a caller must pass the arguments in the
   declared registers (a0, d0, d1 here).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O1" } */

long g (void *p __asm ("a0"), long m __asm ("d0"), long n __asm ("d1"));

long c (void *p) { return g (p, 3, 4); }

/* { dg-final { scan-assembler "moveq #3,d0" } } */
/* { dg-final { scan-assembler "moveq #4,d1" } } */
/* { dg-final { scan-assembler "move.l 4\\(sp\\),a0" } } */
