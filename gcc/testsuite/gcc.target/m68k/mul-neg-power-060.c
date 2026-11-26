/* { dg-do compile } */
/* { dg-options "-O2 -m68060" } */

#include "mul-neg-power-000.c"

/* Larger powers already use shifts.  Preserve those choices while
   correcting negative powers with immediate shift counts.  */
unsigned int
negative2097152 (unsigned int x)
{
  return x * -2097152U;
}

unsigned int
high_bit (unsigned int x)
{
  return x * 0x80000000U;
}

/* { dg-final { scan-assembler-not {__mulsi3|mul[su][.]?[wl]} } } */
