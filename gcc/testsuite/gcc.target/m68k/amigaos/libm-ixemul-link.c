/* -lm must resolve against ixemul, whose libc.a has the math functions.
   The SDK ships no libm.a, so -lm fell through to newlib's libm.a and the
   link failed on _impure_ptr; the toolchain now installs an empty libm.a in
   ixemul/lib.  amigaos.exp runs this file with -mcrt=ixemul in place of the
   board's runtime.  */

/* { dg-do link } */
/* { dg-skip-if "amiga ixemul" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68020 -lm" } */

#include <math.h>

volatile double x = 2.0;

int
main (void)
{
  return sqrt (x) > 1.5 ? 0 : 1;
}
