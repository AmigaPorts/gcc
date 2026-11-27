/* From the 68020 on, divsl.l and divul.l give the quotient and the
   remainder of a / b and a % b, rather than the 68000's divmod call.  */

/* { dg-do compile } */
/* { dg-options "-m68020 -O2" } */

unsigned long
udivmod (unsigned long a, unsigned long b, unsigned long *r)
{
  *r = a % b;
  return a / b;
}

long
sdivmod (long a, long b, long *r)
{
  *r = a % b;
  return a / b;
}

/* { dg-final { scan-assembler-times "divul\\.l" 1 } } */
/* { dg-final { scan-assembler-times "divsl\\.l" 1 } } */
/* { dg-final { scan-assembler-not "__amigaos_u?divmodsi4|__u?divsi3|__u?modsi3" } } */
