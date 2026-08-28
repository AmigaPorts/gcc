/* Amiga / SAS-C register arguments via the regparm attribute: the first two
   arguments are passed in d0/d1 (no explicit __asm bindings).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga regparm ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O1" } */

long __attribute__((regparm (2))) f (long a, long b) { return a + b; }

/* { dg-final { scan-assembler "add.l d1,d0" } } */
