/* Amiga register-parameter ABI: an address-register parameter must be read
   from the declared register (a0 here).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O1" } */

int f (int *p __asm ("a0")) { return *p; }

/* { dg-final { scan-assembler "move.l \\(a0\\),d0" } } */
