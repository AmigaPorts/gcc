/* { dg-do compile } */
/* { dg-options "-O2 -m68030" } */

/* Include the constant source cost when comparing the multiply with a
   shift/add sequence.  */
unsigned int
mul52 (unsigned int x)
{
  return x * 52;
}

/* { dg-final { scan-assembler-not {mul[su][.]?l} } } */
