/* -fPIC with -mpcrel would make the references 32-bit, which ld can't
   resolve across hunks any more than 16-bit ones; it is ignored, and the
   references stay 16-bit.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga pic modes" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68020 -O2 -mpcrel -fPIC" } */
/* { dg-warning "'-fPIC' and '-fPIE' are ignored with '-mpcrel'" "" { target *-*-* } 0 } */

extern int x;

int
get (void)
{
  return x;
}

/* { dg-final { scan-assembler {\(_x:w,pc\)} } } */
/* { dg-final { scan-assembler-not {:l,pc\)} } } */
