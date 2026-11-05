/* { dg-do compile { target powerpc*-*-amigaos* } } */
/* { dg-options "-O2 -mcpu=8548" } */
/* { dg-final { scan-assembler-not "\\tefdcfs " } } */

/* A constant already representable as a double should not require a
   separate conversion instruction after its load.  */
double add_four (double x) { return x + 4.0; }
double zero (void) { return 0.0; }
