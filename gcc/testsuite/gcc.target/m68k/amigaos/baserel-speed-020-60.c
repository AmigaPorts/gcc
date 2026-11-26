/* { dg-do compile } */
/* { dg-options "-O2 -m68020-60 -mregparm=4 -msoft-float -fbaserel -fomit-frame-pointer" } */
/* The 68060's register-load MOVE bypass lets MOVE+ADDI pair.  LEA+MOVE
   cannot use that bypass; retain the larger sequence when tuning for speed.  */
extern int values[16];
int *address (void) { return values; }
/* { dg-final { scan-assembler {move\.l a4,d0\n\tadd\.l #_values:W,d0} } } */
/* { dg-final { scan-assembler-not {lea } } } */
