/* Amiga register-parameter ABI: a _Complex int bound to d0 takes d0 and
   d1, so with -mregparm=3 the arguments after it go in d2 and a0, and
   -mmangle-regparm names the function after those registers.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68000 -mregparm=3 -mmangle-regparm" } */

__attribute__ ((noinline)) int
f (_Complex int c __asm ("d0"), int x, int y)
{
  return __real__ c + x - y;
}

int
g (void)
{
  return f (1 + 2i, 7, 9);
}

/* { dg-final { scan-assembler "\n\"_d0d2a0@f\":" } } */
/* { dg-final { scan-assembler "\tjsr \"_d0d2a0@f\"" } } */
