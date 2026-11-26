/* { dg-do compile } */
/* { dg-options "-O2 -m68020 -mregparm=4 -msoft-float -fbaserel -fomit-frame-pointer" } */
/* Use the smaller LEA+MOVE form for this CPU tune at -O2.  */
extern int values[16];
int *address (void) { return values; }
/* { dg-final { scan-assembler {lea \(_values:W,a4\),a[0-7]} } } */
/* { dg-final { scan-assembler-not {add\.l #_values:W,} } } */
