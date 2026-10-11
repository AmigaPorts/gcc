/* Bindings that do not overlap the frame or stack pointer remain valid.  */
/* { dg-do compile } */
/* { dg-options "-m68000 -O2 -fomit-frame-pointer" } */

void valid_a4 (unsigned long *p __asm("a4")) { p[0] = 1; }
void valid_a6 (unsigned long *p __asm("a6")) { p[0] = 1; }

void valid_pair (unsigned long long x __asm("a3"))
{ __asm__ volatile ("" : : "g" (x)); }

void valid_double (double x __asm("a3"))
{ __asm__ volatile ("" : : "g" (x)); }

void valid_long_double (long double x __asm("a2"))
{ __asm__ volatile ("" : : "g" (x)); }

extern unsigned long valid_call (unsigned long (*fn)(void) __asm("a6"));
extern unsigned long callback (void);

unsigned long valid_caller (unsigned long value)
{
  return valid_call (callback) + value;
}
