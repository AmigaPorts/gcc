/* Amiga register-parameter ABI: mixed data- and address-register parameters
   are each read from their declared registers (d2 and a1 here).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O1" } */

int f (int a __asm ("d2"), int *p __asm ("a1")) { return a + *p; }

/* { dg-final { scan-assembler "move.l d2,d0" } } */
/* { dg-final { scan-assembler "add.l \\(a1\\),d0" } } */
