/* { dg-do compile } */
/* { dg-options "-O1 -fno-inline -fbaserel -mrestore-a4" } */
/* -mrestore-a4 loads a4 in every public function, as if it were __saveds,
   but not in static ones: only code in the same file calls those.  */

int g;

static int helper (void)
{
  return g;
}

int api (void)
{
  return helper () + g;
}

/* { dg-final { scan-assembler-times {jsr ___restore_a4} 1 } } */
