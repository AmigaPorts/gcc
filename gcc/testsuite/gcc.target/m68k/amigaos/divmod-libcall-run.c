/* The 68000's divmod calls, checked against separate / and % calls: signed
   and unsigned, with every combination of signs, divisors of every width,
   and LONG_MIN.  The reference functions see a single operation each, so
   they make the plain __divsi3, __modsi3, __udivsi3 and __umodsi3 calls.  */

/* { dg-do run } */
/* { dg-skip-if "the 68000 has no -fbaserel32" { *-*-* } { "-fbaserel32" "-resident32" } { "" } } */
/* { dg-options "-m68000 -O2" } */

extern void abort (void);

__attribute__ ((noipa)) static unsigned long
udivmod (unsigned long a, unsigned long b, unsigned long *r)
{
  *r = a % b;
  return a / b;
}

__attribute__ ((noipa)) static long
sdivmod (long a, long b, long *r)
{
  *r = a % b;
  return a / b;
}

__attribute__ ((noipa)) static unsigned long
udiv (unsigned long a, unsigned long b)
{
  return a / b;
}

__attribute__ ((noipa)) static unsigned long
umod (unsigned long a, unsigned long b)
{
  return a % b;
}

__attribute__ ((noipa)) static long
sdiv (long a, long b)
{
  return a / b;
}

__attribute__ ((noipa)) static long
smod (long a, long b)
{
  return a % b;
}

static void
check (unsigned long a, unsigned long b)
{
  unsigned long ur;
  long sr;

  if (udivmod (a, b, &ur) != udiv (a, b) || ur != umod (a, b))
    abort ();
  if ((long) b == -1 && (long) a == -2147483647L - 1)
    return;
  if (sdivmod ((long) a, (long) b, &sr) != sdiv ((long) a, (long) b)
      || sr != smod ((long) a, (long) b))
    abort ();
}

int
main (void)
{
  static const unsigned long values[] = {
    0, 1, 2, 3, 7, 10, 0x7fff, 0x8000, 0xffff, 0x10000, 0x1ffff, 123457,
    3000017, 0x01234567, 0x40000001, 0x7fffffff, 0x80000000, 0x8000fffe,
    0x8000ffff, 0xfedcba98, 0xfffffffe, 0xffffffff
  };
  const int n = sizeof (values) / sizeof (values[0]);

  for (int i = 0; i < n; i++)
    for (int j = 1; j < n; j++)
      {
	check (values[i], values[j]);
	check (-values[i], values[j]);
	check (values[i], -values[j]);
	check (-values[i], -values[j]);
      }
  return 0;
}
