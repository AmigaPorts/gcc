/* { dg-do compile } */
/* { dg-options "-O2 -mregparm=4" } */

/* Sibling calls must not be used when -mregparm places arguments in
   callee-saved registers (d2/d3/a2/...), because the sibcall epilogue
   restores those registers over the outgoing arguments.  */

int callee4 (int a, int b, int c, int d);
int callee2 (int a, int b);

int
caller4 (int a, int b, int c, int d)
{
  return callee4 (a, b, c + 1, d);
}

int
caller2 (int a, int b)
{
  return callee2 (a, b + 1);
}

/* Four-arg call uses d2/d3: must be a normal call.  */
/* { dg-final { scan-assembler "j(b|)sr\[ \t\]*callee4" } } */
/* { dg-final { scan-assembler-not "j(ra|mp)\[ \t\]*callee4" } } */

/* Two-arg call uses only d0/d1: sibcall is still allowed.  */
/* { dg-final { scan-assembler "j(ra|mp)\[ \t\]*callee2" } } */
