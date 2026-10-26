/* { dg-do compile } */
/* { dg-options "-O1 -fno-inline -fbaserel -malways-restore-a4" } */
/* -malways-restore-a4 loads a4 in every function, static ones included.  */

int g;

static int helper (void)
{
  return g;
}

int api (void)
{
  return helper () + g;
}

/* { dg-final { scan-assembler-times {jsr ___restore_a4} 2 } } */
