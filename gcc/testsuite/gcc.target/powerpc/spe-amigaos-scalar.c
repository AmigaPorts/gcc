/* { dg-do compile { target powerpc*-*-amigaos* } } */
/* { dg-options "-mcpu=8548 -O2" } */
/* { dg-final { scan-assembler "efdadd" } } */
/* { dg-final { scan-assembler "efscfsi" } } */
/* { dg-final { scan-assembler-not "fmadd|fmsub" } } */

double add (double a, double b) { return a + b; }
float from_int (int a) { return a; }
double multiply_add (double a, double b, double c) { return a * b + c; }

/* Reload must not treat a folded subreg of memory as a register pair.  */
double
large_frame (double value)
{
  volatile double buffer[128];
  for (int i = 0; i < 128; ++i)
    buffer[i] = value + i;
  return buffer[99];
}

int unordered (double a, double b) { return __builtin_isunordered (a, b); }

/* if-conversion must select the GT bit used by SPE comparisons.  */
int select_eq (double a, double b, int yes, int no)
{
  return a == b ? yes : no;
}
