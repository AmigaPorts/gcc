/* The truncation patterns print the low part of a pc-relative source as
   sym+offset; that address must not be validated, since a pc-relative
   address is only legitimate in a source operand.  The call gives each
   function a second basic block, so that the truncation reaches final
   instead of being narrowed into a plain move.
   AmigaPorts/m68k-amigaos-gcc#409.  */

/* { dg-do compile } */
/* { dg-options "-O2 -m68020 -mpcrel" } */

int gi;
short gs;

void g (void);

void
st_hi (short *p, int k)
{
  if (k)
    g ();
  *p = gi;
}

void
st_qi (char *p, int k)
{
  if (k)
    g ();
  *p = gi;
}

void
st_qi_hi (char *p, int k)
{
  if (k)
    g ();
  *p = gs;
}

/* { dg-final { scan-assembler "move\\.w \\(_gi\\+2(:w)?,pc\\)" } } */
/* { dg-final { scan-assembler "move\\.b \\(_gi\\+3(:w)?,pc\\)" } } */
/* { dg-final { scan-assembler "move\\.b \\(_gs\\+1(:w)?,pc\\)" } } */
