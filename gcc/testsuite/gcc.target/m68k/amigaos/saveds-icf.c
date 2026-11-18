/* { dg-do compile } */
/* { dg-options "-O2 -fbaserel" } */
/* Identical code folding does not make a __saveds function an alias of a
   normal one that has the same body: called from outside the program, it
   would run without loading a4.  */

int g;

__attribute__ ((noinline)) static int
normal (int x)
{
  return g + x * 3;
}

__attribute__ ((noinline)) static __saveds int
hook (int x)
{
  return g + x * 3;
}

int (*volatile p) (int) = hook;

int
use (int x)
{
  return normal (x) + hook (x);
}

/* { dg-final { scan-assembler-times {jsr ___restore_a4} 1 } } */
/* { dg-final { scan-assembler-not {\.set\s+_hook,} } } */
