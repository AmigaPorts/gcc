/* { dg-do compile } */
/* { dg-options "-O2 -m68040 -m68881 -fexcess-precision=standard" } */
/* libgcc's ___floatdidf and ___floatundidf return the extended value, so a
   long long converts to long double, and fdmove rounds it to double in the
   register.  int to float needs nothing: fsmove rounds.  */

double
di_df (long long i)
{
  return (double) i;
}

double
udi_df (unsigned long long i)
{
  return (double) i;
}

float
si_sf (int i)
{
  return (float) i;
}

/* { dg-final { scan-assembler-not {___float(un)?didf} } } */
/* { dg-final { scan-assembler-times {fdmove\.x} 2 } } */
/* { dg-final { scan-assembler {fsmove\.l} } } */
