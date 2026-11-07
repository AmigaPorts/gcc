/* { dg-do compile } */
/* { dg-options "-O2 -m68020 -fno-inline -fbaserel32 -mrestore-a4" } */
/* -mrestore-a4 loads a4 in a public function and gives the caller's back
   in its epilogue, so a tail call from it is a call: a sibcall would enter
   bar with the caller's a4.  The static helper keeps its sibcall.  */

int bar (int x);

static int
helper (int x)
{
  return bar (x + 1);
}

int
api (int x)
{
  return bar (x);
}

int
api2 (int x)
{
  return helper (x) + 1;
}

/* { dg-final { scan-assembler-times {(jsr|bsr\.l) _bar} 1 } } */
/* { dg-final { scan-assembler-times {(bra\.l|jra|jmp) _bar} 1 } } */
