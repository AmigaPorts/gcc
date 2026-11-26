/* { dg-do compile } */
/* { dg-options "-O2 -m68030 -mregparm=4 -msoft-float -fbaserel -fomit-frame-pointer" } */
/* With cached instructions, the 68030 takes eight cycles for MOVE+ADDI.L,
   versus six for LEA+MOVE.  This is also a speed optimization.  */
extern int values[16];
int *address (void) { return values; }
/* { dg-final { scan-assembler {lea \(_values:W,a4\),a[0-7]} } } */
/* { dg-final { scan-assembler-not {add\.l #_values:W,} } } */
