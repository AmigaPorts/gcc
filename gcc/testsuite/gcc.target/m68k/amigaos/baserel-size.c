/* { dg-do compile } */
/* { dg-options "-Os -m68000 -mregparm=4 -msoft-float -fbaserel -fomit-frame-pointer" } */
/* Materializing a base-relative address with move.l a4,Rn and an
   immediate add takes eight bytes.  LEA takes four, or six with a move
   from a free address register to the pointer-result register d0.  */
extern int values[16];
extern void consume (int *);

int *
address (void)
{
  return values;
}

void
argument (void)
{
  consume (values);
}

/* { dg-final { scan-assembler-not {add\.l #_values:W,} } } */
/* { dg-final { scan-assembler-times {lea \(_values:W,a4\),a[0-7]} 2 } } */
