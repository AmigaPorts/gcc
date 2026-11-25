/* { dg-do compile } */
/* { dg-options "-Os -m68030 -fbaserel -mregparm=4 -msoft-float" } */
/* A large addend must not be narrowed to a d16 displacement.  */
extern char values[];
char *address (void) { return values + 40000; }
/* { dg-final { scan-assembler-not {lea \(_values\+40000:W,a4\),} } } */
