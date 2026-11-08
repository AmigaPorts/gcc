/* { dg-do compile } */
/* { dg-options "-O2 -m68020 -fno-inline -resident -malways-restore-a4" } */
/* -resident is -fbaserel to the compiler: under -malways-restore-a4 every
   function loads a4 and gives the caller's back, so none of them may
   leave through a sibcall, static ones included.  */

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

/* { dg-final { scan-assembler-times {(jsr|bsr\.l) _bar} 2 } } */
/* { dg-final { scan-assembler-not {(bra\.l|jra|jmp) _bar} } } */
