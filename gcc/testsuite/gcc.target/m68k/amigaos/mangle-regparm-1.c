/* { dg-do compile } */
/* { dg-options "-O2 -m68000 -mregparm=2 -mmangle-regparm" } */

/* -mmangle-regparm names a function after the registers its arguments
   take: @name when they are those of a SAS/C __regargs function, else
   _<registers>@name with "s" for a stack argument before the last
   register one.  Gas takes the @ only in quotes.  */

typedef unsigned char *ptr;

#define NI __attribute__ ((noinline))

/* d0 and a0, as SAS/C passes them.  */
NI int sas (int a, ptr p) { return a + *p; }
/* { dg-final { scan-assembler "\n\"@sas\":" } } */

/* The third integer goes to a0, where SAS/C would use the stack.  */
NI int alt (int a, int b, int c) { return a + b + c; }
/* { dg-final { scan-assembler "\n\"_d0d1a0@alt\":" } } */

/* Explicit registers.  */
NI int regs (register int a __asm ("d2"), int b, register ptr p __asm ("a1"))
{
  return a + b + *p;
}
/* { dg-final { scan-assembler "\n\"_d2d0a1@regs\":" } } */

/* All on the stack, variadic, no arguments, an asm label: the names stay.  */
NI __attribute__ ((stkparm)) int stk (int a, ptr p) { return a + *p; }
NI int var (int a, ...) { return a; }
NI int none (void) { return 1; }
NI int labelled (int a, ptr p) __asm__ ("_labelled_by_user");
NI int labelled (int a, ptr p) { return a - *p; }
/* { dg-final { scan-assembler "\n_stk:" } } */
/* { dg-final { scan-assembler "\n_var:" } } */
/* { dg-final { scan-assembler "\n_none:" } } */
/* { dg-final { scan-assembler "\n_labelled_by_user:" } } */

/* Calls use the same names.  */
int
call (int a, ptr p)
{
  return sas (a, p) + alt (a, a, a) + regs (a, a, p) + stk (a, p)
	 + var (a, a) + none () + labelled (a, p);
}
/* { dg-final { scan-assembler "\tjsr \"@sas\"" } } */
/* { dg-final { scan-assembler "\tjsr \"_d0d1a0@alt\"" } } */
/* { dg-final { scan-assembler "\tjsr \"_d2d0a1@regs\"" } } */
/* { dg-final { scan-assembler "\tjsr _stk" } } */
