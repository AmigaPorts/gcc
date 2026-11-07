/* Amiga register-parameter ABI: a complex double takes two FP registers,
   so fp7 cannot hold one and fp6 takes fp6 and fp7.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68040 -m68881" } */

_Complex double
f7 (_Complex double c __asm ("fp7"))
/* { dg-error "register .fp7. cannot hold" "" { target *-*-* } .-1 } */
{
  return c;
}

_Complex double
f6 (_Complex double c __asm ("fp6"))
{
  return c;
}
