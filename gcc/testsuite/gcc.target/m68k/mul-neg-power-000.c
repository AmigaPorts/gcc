/* { dg-do compile } */
/* { dg-options "-O2 -m68000" } */

/* Do not price a negative power of two as an already reduced shift.  */
unsigned int
negative2 (unsigned int x)
{
  return x * -2U;
}

unsigned int
negative128 (unsigned int x)
{
  return x * -128U;
}

/* { dg-final { scan-assembler-not {__mulsi3|mul[su][.]?[wl]} } } */
