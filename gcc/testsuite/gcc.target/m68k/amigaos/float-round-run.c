/* { dg-do run } */
/* { dg-options "-fexcess-precision=standard" } */
/* A conversion from an integer wider than the mantissa rounds to the type
   it converts to, once, and before any operation it is an operand of.  A
   68881 cannot round in a register, and the value was returned with the
   precision of long double.  */

__attribute__ ((noipa)) float
si_sf (int i)
{
  return (float) i;
}

__attribute__ ((noipa)) float
usi_sf (unsigned i)
{
  return (float) i;
}

__attribute__ ((noipa)) double
usi_df (unsigned i)
{
  return (double) i;
}

__attribute__ ((noipa)) float
uhi_sf (unsigned short i)
{
  return (float) i;
}

__attribute__ ((noipa)) float
uqi_sf (unsigned char i)
{
  return (float) i;
}

__attribute__ ((noipa)) float
di_sf (long long i)
{
  return (float) i;
}

__attribute__ ((noipa)) float
udi_sf (unsigned long long i)
{
  return (float) i;
}

__attribute__ ((noipa)) double
di_df (long long i)
{
  return (double) i;
}

__attribute__ ((noipa)) double
udi_df (unsigned long long i)
{
  return (double) i;
}

/* fadd.l and the others would add the exact int, and round only the sum.  */

__attribute__ ((noipa)) float
add_if (float a, int i)
{
  return a + (float) i;
}

__attribute__ ((noipa)) float
sub_if (float a, int i)
{
  return a - (float) i;
}

__attribute__ ((noipa)) float
mul_if (float a, int i)
{
  return a * (float) i;
}

__attribute__ ((noipa)) float
div_if (float a, int i)
{
  return a / (float) i;
}

/* 2^60 + 2^36 + 1: to float directly, above the halfway point, so up; to
   double first, 2^60 + 2^36, halfway, so to even.  */
#define WIDE ((1LL << 60) + (1LL << 36) + 1)

int
main (void)
{
  if (si_sf (0x7fffffff) != 0x1p31f || si_sf (16777217) != 16777216.0f
      || si_sf (-16777217) != -16777216.0f || si_sf (16777219) != 16777220.0f)
    return 1;
  if (usi_sf (0xffffffffu) != 0x1p32f || usi_sf (16777217u) != 16777216.0f
      || usi_sf (0x80000000u) != 0x1p31f)
    return 2;
  /* Ties to even above 2^31, where a float's ulp is 256.  */
  if (usi_sf (0x80000080u) != 0x1p31f || usi_sf (0x80000081u) != 0x1.000002p31f
      || usi_sf (0x80000180u) != 0x1.000004p31f)
    return 3;
  if (usi_df (0xffffffffu) != 4294967295.0 || usi_df (7u) != 7.0)
    return 4;
  if (uhi_sf (0xffff) != 65535.0f || uqi_sf (0xff) != 255.0f)
    return 5;
  if (di_sf (16777217) != 16777216.0f || di_sf (WIDE) != 0x1.000002p60f
      || di_sf (-WIDE) != -0x1.000002p60f)
    return 6;
  if (udi_sf (~0ULL) != 0x1p64f || udi_sf (WIDE) != 0x1.000002p60f)
    return 7;
  if (di_df (0x7fffffffffffffffLL) != 0x1p63
      || di_df (-0x7fffffffffffffffLL) != -0x1p63)
    return 8;
  if (udi_df (~0ULL) != 0x1p64)
    return 9;
  if (add_if (1.0f, 16777217) != 16777216.0f
      || sub_if (1.0f, 16777217) != -16777215.0f
      || mul_if (3.0f, 16777217) != 50331648.0f
      || div_if (16777216.0f, 16777217) != 1.0f)
    return 10;
  return 0;
}
