/* { dg-do compile } */
/* { dg-options "-O2 -m68020-60 -m68881 -fexcess-precision=standard" } */
/* An unsigned short or char converts as the int or short it zero-extends
   to, which is exact: no test of the sign and no 2^32 added, as for an
   unsigned int, no rounding through memory, and no call to libgcc.  */

float
uhi_sf (unsigned short i)
{
  return (float) i;
}

double
uhi_df (unsigned short i)
{
  return (double) i;
}

float
uqi_sf (unsigned char i)
{
  return (float) i;
}

double
uqi_df (unsigned char i)
{
  return (double) i;
}

/* { dg-final { scan-assembler-not {fadd} } } */
/* { dg-final { scan-assembler-not {___float} } } */
/* { dg-final { scan-assembler-not {fmove\.s} } } */
/* { dg-final { scan-assembler-times {fmove\.[lw]} 4 } } */
