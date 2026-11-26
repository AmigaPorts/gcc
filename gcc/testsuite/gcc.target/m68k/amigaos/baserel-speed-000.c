/* { dg-do compile } */
/* { dg-options "-O2 -m68000 -mregparm=4 -msoft-float -fbaserel -fomit-frame-pointer" } */
/* The 68000 has no dual-issue tradeoff: LEA+MOVE is shorter and faster
   than MOVE+ADDI.L, so use it when optimizing for speed too.  */
extern int values[16];
int *address (void) { return values; }
/* { dg-final { scan-assembler {lea \(_values:W,a4\),a[0-7]} } } */
/* { dg-final { scan-assembler-not {add\.l #_values:W,} } } */
