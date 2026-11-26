/* { dg-do compile } */
/* { dg-options "-O2 -m68060 -mregparm=4 -msoft-float -fbaserel -fomit-frame-pointer" } */
/* An address-register result uses one LEA even on the 68060.  */
extern int values[16];
int
load (void)
{
  register int *p __asm__ ("a0") = values;
  __asm__ ("" : "+a" (p));
  return *p;
}
/* { dg-final { scan-assembler {lea \(_values:W,a4\),a0} } } */
/* { dg-final { scan-assembler-not {add\.l #_values:W,} } } */
