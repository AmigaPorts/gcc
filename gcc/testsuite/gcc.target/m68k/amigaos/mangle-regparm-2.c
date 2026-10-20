/* { dg-do compile } */
/* { dg-options "-O2 -m68000 -mmangle-regparm" } */

/* Without -mregparm only explicit registers mangle a name: a stack
   argument before the last register one is "s".  */

typedef unsigned char *ptr;

int plain (int a, ptr p) { return a + *p; }
/* { dg-final { scan-assembler "\n_plain:" } } */

int regs (register int a __asm ("d2"), int b, register ptr p __asm ("a1"))
{
  return a + b + *p;
}
/* { dg-final { scan-assembler "\n\"_d2sa1@regs\":" } } */

/* A regparm attribute that matches SAS/C.  */
__attribute__ ((regparm (2))) int attr (int a, ptr p) { return a + *p; }
/* { dg-final { scan-assembler "\n\"@attr\":" } } */
