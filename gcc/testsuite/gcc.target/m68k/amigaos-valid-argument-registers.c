/* A6 is a valid Amiga argument register; reject only overlapping bindings.  */
/* { dg-do compile } */
/* { dg-skip-if "Amiga register parameters" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -mregparm=3 -fomit-frame-pointer" } */
void valid_a4 (unsigned long *p __asm("a4")) { p[0] = 1; }
void valid_a6 (unsigned long *p __asm("a6")) { p[0] = 1; }
void valid_pair (unsigned long long x __asm("a3"))
{ __asm__ volatile ("" : : "g" (x)); }
void valid_double (double x __asm("a3"))
{ __asm__ volatile ("" : : "g" (x)); }
void valid_long_double (long double x __asm("a2"))
{ __asm__ volatile ("" : : "g" (x)); }
