/* { dg-do compile } */
/* { dg-options "-O2 -m68040" } */

/* The 040 table needs separate tuning: adding its constant source cost
   selects a longer shift/add sequence for this factor.  */
unsigned int
mul262151 (unsigned int x)
{
  return x * 262151;
}

/* { dg-final { scan-assembler {mul[su][.]?l} } } */
