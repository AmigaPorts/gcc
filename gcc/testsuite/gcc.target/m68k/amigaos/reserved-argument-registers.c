/* Reject explicit argument registers overlapping the frame pointer, and
   multi-register arguments spilling into the stack pointer, including
   callers that see only the register-bound prototype.  */
/* { dg-do compile } */
/* { dg-options "-m68000 -O2 -fomit-frame-pointer" } */

void bad_a5 (unsigned long *p __asm("a5")) /* { dg-error "reserved for the frame or stack pointer" } */
{ p[0] = 1; }

void bad_pair (unsigned long long value __asm("a4")) /* { dg-error "reserved for the frame or stack pointer" } */
{ __asm__ volatile ("" : : "g" (value)); }

void bad_double (double value __asm("a4")) /* { dg-error "reserved for the frame or stack pointer" } */
{ __asm__ volatile ("" : : "g" (value)); }

void bad_long_double (long double value __asm("a3")) /* { dg-error "reserved for the frame or stack pointer" } */
{ __asm__ volatile ("" : : "g" (value)); }

void bad_stack_pair (double value __asm("a6")) /* { dg-error "reserved for the frame or stack pointer" } */
{ __asm__ volatile ("" : : "g" (value)); }

extern unsigned long Super (unsigned long (*fn)(void) __asm("a5"));
extern unsigned long callback (void);

unsigned long bad_caller (unsigned long value)
{
  unsigned long local = value;
  return Super (callback) + local; /* { dg-error "reserved for the frame or stack pointer" } */
}

/* An SP parameter only reads the entry stack pointer: a caller must not
   load an argument into SP.  */
extern void sp_entry (int sp __asm("sp"));

void bad_sp_caller (int value)
{
  sp_entry (value); /* { dg-error "reserved for the frame or stack pointer" } */
}
