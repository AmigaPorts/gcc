/* { dg-do compile } */
/* { dg-options "-O2 -m68060 -mregparm=4 -msoft-float -fbaserel -fomit-frame-pointer" } */
/* Cold code uses the size-saving form even with global -O2.  */
extern int values[16];
__attribute__ ((cold)) int *
address (void)
{
  return values;
}
/* { dg-final { scan-assembler {lea \(_values:W,a4\),a[0-7]} } } */
/* { dg-final { scan-assembler-not {add\.l #_values:W,} } } */
