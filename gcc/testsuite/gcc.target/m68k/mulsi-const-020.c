/* { dg-do compile } */
/* { dg-options "-O2 -m68020" } */

/* Include the constant source cost when comparing with shifts and adds.  */
unsigned int
mul134 (unsigned int x)
{
  return x * 134;
}

/* { dg-final { scan-assembler-not {mul[su][.]?l} } } */
