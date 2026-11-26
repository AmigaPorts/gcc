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

unsigned int
negative256 (unsigned int x)
{
  return x * -256U;
}

#if !defined (__mc68060__)
unsigned int
negative512 (unsigned int x)
{
  return x * -512U;
}

unsigned int
negative65536 (unsigned int x)
{
  return x * -65536U;
}
#endif

/* { dg-final { scan-assembler-not {__mulsi3|mul[su][.]?[wl]} } } */
