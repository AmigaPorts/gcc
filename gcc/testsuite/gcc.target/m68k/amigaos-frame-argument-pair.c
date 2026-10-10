/* Multi-register arguments must not overlap the eliminated frame register.  */
/* { dg-do compile } */
/* { dg-skip-if "Amiga register parameters" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -mregparm=3 -fomit-frame-pointer" } */
void bad_pair (unsigned long long value __asm("a4")) /* { dg-error "reserved for the frame or stack pointer" } */
{ __asm__ volatile ("" : : "g" (value)); }
void bad_double (double value __asm("a4")) /* { dg-error "reserved for the frame or stack pointer" } */
{ __asm__ volatile ("" : : "g" (value)); }
void bad_long_double (long double value __asm("a3")) /* { dg-error "reserved for the frame or stack pointer" } */
{ __asm__ volatile ("" : : "g" (value)); }
void bad_stack_pair (double value __asm("a6")) /* { dg-error "reserved for the frame or stack pointer" } */
{ __asm__ volatile ("" : : "g" (value)); }
