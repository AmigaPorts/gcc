/* { dg-do compile } */
/* { dg-options "-O2 -m68000" } */

/* For speed, a 32-bit counter keeps dbra and borrows into the high word
   when the low word runs out.  The borrow ends the loop only when the
   whole counter was 0 (jcc), not when its high word turns negative.  */

volatile int sink;

void
fill (unsigned long n)
{
  while (n--)
    sink = 1;
}

/* Leaves on a test as well: the dbCC form.  */
static const int ref[5] = { 12, 14, 16, 18, 10 };

void
verify (const int *a, const int *b)
{
  for (int i = 0; i < 5; i++)
    {
      if (a[i] != ref[i])
	__builtin_abort ();
      if (b[i])
	__builtin_abort ();
    }
}

/* { dg-final { scan-assembler {\tdbra d[0-7],} } } */
/* { dg-final { scan-assembler {\tdbne d[0-7],} } } */
/* { dg-final { scan-assembler-times {clr\.w d[0-7]\n\tsubq\.l #1,d[0-7]\n\tjcc } 2 } } */
/* { dg-final { scan-assembler-not {\tjpl } } } */
