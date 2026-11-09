/* Amiga register-parameter ABI: a struct with no integer mode cannot be
   bound to a register, and the diagnostics say so instead of crashing.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68020" } */

struct S3 { int a, b, c; };

int
f (struct S3 s __asm ("d0")) /* { dg-error "register .d0. cannot hold" } */
{
  return s.a;
}

int
g (int x __asm ("x9")) /* { dg-error "invalid register name" } */
{
  return x;
}
