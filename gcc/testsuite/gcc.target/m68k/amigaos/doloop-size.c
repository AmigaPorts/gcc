/* { dg-do compile } */
/* { dg-options "-Os -m68000" } */

/* A 32-bit dbra needs clr.w, subq.l and jcc for the borrow into the high
   word: 10 bytes.  subq.l and jcc are 4 and count all 32 bits.  */

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

/* { dg-final { scan-assembler-times {subq\.l #1,d[0-7]\n\tjcc } 2 } } */
/* { dg-final { scan-assembler-not {\tdb[a-z]+ } } } */
