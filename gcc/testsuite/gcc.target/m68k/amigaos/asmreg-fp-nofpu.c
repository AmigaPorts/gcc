/* Amiga register-parameter ABI: without an FPU nothing can move a value
   in or out of an FP register, so binding an argument to one is an error,
   in the definition and in a call, not an internal compiler error.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68040" } */
/* { dg-skip-if "has an FPU" { *-*-* } { "-m68881" "-mhard-float" } } */

extern double ext_df (double a __asm ("fp0"));

double
def_df (double a __asm ("fp1")) /* { dg-error "register .fp1. cannot hold" } */
{
  return a;
}

double
call_df (void)
{
  return ext_df (1.5); /* { dg-error "register .fp0. cannot hold" } */
}
