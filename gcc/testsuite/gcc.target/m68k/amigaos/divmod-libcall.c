/* On the 68000, a / b and a % b of the same operands are one call that
   returns both, signed or unsigned.  A constant divisor keeps its own
   expansion, and from the 68020 on divsl.l and divul.l give both.  */

/* { dg-do compile } */
/* { dg-skip-if "the 68000 has no -fbaserel32" { *-*-* } { "-fbaserel32" "-resident32" } { "" } } */
/* { dg-options "-m68000 -O2" } */

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

unsigned long
udivmod10 (unsigned long a, unsigned long *r)
{
  *r = a % 10;
  return a / 10;
}

/* { dg-final { scan-assembler-times "___amigaos_udivmodsi4" 1 } } */
/* { dg-final { scan-assembler-times "___amigaos_divmodsi4" 1 } } */
/* { dg-final { scan-assembler-times "___udivsi3" 1 } } */
/* { dg-final { scan-assembler-times "___umodsi3" 1 } } */
/* { dg-final { scan-assembler-not "___divsi3|___modsi3" } } */
