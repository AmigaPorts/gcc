/* { dg-do compile } */
/* { dg-options "-O2 -m68020-60 -m68881 -fexcess-precision=standard" } */
/* An unsigned int converts to float and double inline: it does not widen
   to long long for the conversions that go through libgcc.  */

float
usi_sf (unsigned i)
{
  return (float) i;
}

double
usi_df (unsigned i)
{
  return (double) i;
}

/* { dg-final { scan-assembler-not {___float(un)?di} } } */
/* { dg-final { scan-assembler-times {fmove\.l} 2 } } */
