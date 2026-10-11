/* subdi_dishl32 accepted a memory destination together with a memory
   source and emitted a memory-to-memory sub.l, which the assembler
   rejects.  */
/* { dg-do run } */
/* { dg-options "-O2" } */

extern void abort (void);

__attribute__((noipa)) void
f (unsigned long long *p, unsigned long long *q)
{
  *p -= *q << 32;
}

int
main (void)
{
  unsigned long long a = 0x0000000500000007ULL;
  unsigned long long b = 0x1234567800000002ULL;
  f (&a, &b);
  if (a != 0x0000000300000007ULL)
    abort ();
  return 0;
}
