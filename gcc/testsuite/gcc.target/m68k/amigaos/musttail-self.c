/* A tail call to the current function is set up in the callee-side
   cumulative args (mycum), so the register-parameter sibcall check
   compared against stale data and refused it; musttail then errored
   out.  -O0 keeps the tail recursion from being turned into a loop.  */
/* { dg-do compile } */
/* { dg-options "-O0" } */
/* { dg-final { scan-assembler-not "jsr\[ \t\]+_f2" } } */

void f2 (int n)
{
  if (n)
    [[gnu::musttail]] return f2 (n - 1);
}
