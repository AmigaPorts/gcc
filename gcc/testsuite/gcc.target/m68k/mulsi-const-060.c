/* { dg-do compile } */
/* { dg-options "-O2 -m68060" } */

/* Preserve the faster multiply on 060 rather than applying the 020/030
   constant source cost adjustment to its separate table.  */
unsigned int
mul65537 (unsigned int x)
{
  return x * 65537;
}

/* { dg-final { scan-assembler {mul[su][.]?l} } } */
