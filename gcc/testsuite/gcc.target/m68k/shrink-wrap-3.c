/* The stack-argument offsets are right on both sides of a shrink-wrapped
   prologue, with and without the stack-argument extension.  */
/* { dg-do run } */
/* { dg-options "-Os" } */

struct big { unsigned v[6]; };

unsigned flag;

__attribute__ ((noipa)) unsigned
sum (const unsigned *p, unsigned n)
{
  unsigned s = 0;
  while (n--)
    s += *p++;
  return s;
}

__attribute__ ((noipa)) unsigned
g (struct big b, unsigned x, unsigned long long w)
{
  if (flag == 1)
    return b.v[5] + x;
  if (flag == 2)
    return (unsigned) (w >> 32) + b.v[0];
  unsigned t[8];
  for (unsigned i = 0; i < 8; i++)
    t[i] = b.v[i % 6] * (i + x);
  return sum (t, 8) + sum (b.v, 6) + (unsigned) w;
}

__attribute__ ((noipa, optimize ("no-shrink-wrap"))) unsigned
ref (struct big b, unsigned x, unsigned long long w)
{
  if (flag == 1)
    return b.v[5] + x;
  if (flag == 2)
    return (unsigned) (w >> 32) + b.v[0];
  unsigned t[8];
  for (unsigned i = 0; i < 8; i++)
    t[i] = b.v[i % 6] * (i + x);
  return sum (t, 8) + sum (b.v, 6) + (unsigned) w;
}

int
main (void)
{
  struct big b = { { 3, 5, 7, 11, 13, 17 } };
  for (flag = 0; flag < 3; flag++)
    if (g (b, 9, 0x1234567800000042ULL) != ref (b, 9, 0x1234567800000042ULL))
      __builtin_abort ();
  return 0;
}
