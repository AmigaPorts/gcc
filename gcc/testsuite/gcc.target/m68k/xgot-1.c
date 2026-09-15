/* { dg-do compile } */
/* { dg-skip-if "no GOT on amigaos" { m68k-*-amigaos* } } */
/* { dg-options "-O2 -fpic -mxgot -mcpu=5206" } */
/* { dg-final { scan-assembler "foo@GOT,\%\[ad\]\[0-7\]" } } */

extern int foo;

int
bar (void)
{
  return foo;
}
