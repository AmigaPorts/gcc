/* { dg-do run } */
/* { dg-options "-fexcess-precision=standard" } */
/* A conversion from an integer wider than the mantissa rounds to the type
   it converts to.  A 68881 cannot round in a register, and the value was
   returned with the precision of long double.  */

__attribute__ ((noipa)) float
si_sf (int i)
{
  return (float) i;
}

__attribute__ ((noipa)) float
di_sf (long long i)
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

int
main (void)
{
  if (usi_sf (0xffffffffu) != 0x1p32f || usi_sf (16777217u) != 16777216.0f
      || usi_sf (0x80000000u) != 0x1p31f)
    return 5;
  if (usi_df (0xffffffffu) != 4294967295.0 || usi_df (7u) != 7.0)
    return 6;
  if (si_sf (0x7fffffff) != 0x1p31f || si_sf (16777217) != 16777216.0f)
    return 1;
  if (di_sf (16777217) != 16777216.0f)
    return 2;
  if (di_df (0x7fffffffffffffffLL) != 0x1p63)
    return 3;
  if (udi_df (~0ULL) != 0x1p64)
    return 4;
  return 0;
}
