/* -mpcrel with base-relative data used to emit an a4-relative offset as a
   pc-relative operand, add.l #(_a:L:l,pc),a0, which gas rejects; until the
   combination has a meaning, it is an error.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga pic modes" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-mpcrel -fbaserel" } */
/* { dg-error "'-mpcrel' and '-fbaserel' or '-resident' are mutually exclusive" "" { target *-*-* } 0 } */

int a;

void
set (int i)
{
  a = i;
}
