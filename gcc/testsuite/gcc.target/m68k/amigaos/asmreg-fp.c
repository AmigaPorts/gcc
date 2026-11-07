/* Amiga register-parameter ABI: an FP register holds a float argument of
   any mode on its own, so fp0 and fp1 carry two doubles or two long
   doubles, in the definition and in a call.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68040 -m68881" } */

float
add_sf (float a __asm ("fp0"), float b __asm ("fp1"))
{
  return a + b;
}

double
add_df (double a __asm ("fp0"), double b __asm ("fp1"))
{
  return a + b;
}

long double
add_xf (long double a __asm ("fp0"), long double b __asm ("fp1"))
{
  return a + b;
}

extern double ext_df (double a __asm ("fp0"), double b __asm ("fp1"));

double
call_df (void)
{
  return ext_df (1.5, 2.25);
}

/* { dg-final { scan-assembler {\tfsadd\.x fp1,fp0} } } */
/* { dg-final { scan-assembler {\tfdadd\.x fp1,fp0} } } */
/* { dg-final { scan-assembler {\tfadd\.x fp1,fp0} } } */
/* { dg-final { scan-assembler {fmove\.d #0x3ff8000000000000,fp0} } } */
/* { dg-final { scan-assembler {fmove\.d #0x4002000000000000,fp1} } } */
/* { dg-final { scan-assembler {jsr _ext_df} } } */
