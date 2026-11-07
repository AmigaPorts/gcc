/* { dg-do compile } */
/* { dg-options "-O2 -m68020 -fbaserel" } */
/* A __saveds function gives the caller's a4 back in its epilogue, so a
   sibcall from it would enter bar with the caller's a4: it calls bar and
   returns.  A function without __saveds keeps its sibcall.  */

int bar (int x);

__saveds int
hook (int x)
{
  return bar (x);
}

int
plain (int x)
{
  return bar (x + 1);
}

/* { dg-final { scan-assembler-times {(jsr|bsr\.l) _bar} 1 } } */
/* { dg-final { scan-assembler-times {(bra\.l|jra|jmp) _bar} 1 } } */
