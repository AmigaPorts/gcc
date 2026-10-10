/* The target diagnostic also applies to C++ register parameters.  */
/* { dg-do compile } */
/* { dg-skip-if "Amiga register parameters" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -mregparm=3 -fomit-frame-pointer" } */
void bad_a5 (unsigned long *p __asm("a5")) /* { dg-error "reserved for the frame or stack pointer" } */
{ p[0] = 1; }
void bad_pair (unsigned long long x __asm("a4")) /* { dg-error "reserved for the frame or stack pointer" } */
{ __asm__ volatile ("" : : "g" (x)); }
void bad_double (double x __asm("a4")) /* { dg-error "reserved for the frame or stack pointer" } */
{ __asm__ volatile ("" : : "g" (x)); }
void valid_a6 (unsigned long *p __asm("a6")) { p[0] = 1; }
