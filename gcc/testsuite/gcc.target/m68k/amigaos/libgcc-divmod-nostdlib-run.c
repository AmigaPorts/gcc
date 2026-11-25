/* libgcc's 68000 division helpers, checked against a shift-and-subtract
   division for divisors of every width and the cases where the quotient
   estimate is one too large: divisors whose low bits are all ones, with
   dividends next to their multiples.  -nostdlib, so that the helpers are
   libgcc's and not a C library's.

   No startup code: the jump goes first, and start's return value is the
   program's exit status.  */

/* { dg-do run } */
/* { dg-skip-if "a4 needs the startup code, and the 68000 has no -fbaserel32" { *-*-* } { "-fbaserel*" "-resident*" } { "" } } */
/* { dg-options "-m68000 -O2 -nostdlib -lgcc" } */

__asm__ ("\t.text\n\tjra _start");

static unsigned long
ref (unsigned long n, unsigned long d, unsigned long *r)
{
  unsigned long q = 0, rem = 0;

  for (int i = 31; i >= 0; i--)
    {
      unsigned long carry = rem >> 31;

      rem = (rem << 1) | ((n >> i) & 1);
      if (carry || rem >= d)
	{
	  rem -= d;
	  q |= 1UL << i;
	}
    }
  *r = rem;
  return q;
}

static int
check (unsigned long n, unsigned long d)
{
  volatile unsigned long vn = n, vd = d;
  volatile long sn = (long) n, sd = (long) d;
  unsigned long r, q = ref (n, d, &r);

  if (vn / vd != q || vn % vd != r)
    return 1;
  if (sd == -1 && sn == -2147483647L - 1)
    return 0;

  unsigned long an = sn < 0 ? -n : n, ad = sd < 0 ? -d : d, ar;
  unsigned long aq = ref (an, ad, &ar);
  long wq = (sn < 0) != (sd < 0) ? -(long) aq : (long) aq;
  long wr = sn < 0 ? -(long) ar : (long) ar;

  return sn / sd != wq || sn % sd != wr;
}

static unsigned long seed = 425;

static unsigned long
next (void)
{
  seed = seed * 1103515245UL + 12345;
  return (seed >> 16) | (seed << 16);
}

__attribute__ ((used)) long
start (void)
{
  static const unsigned short tops[] = { 0x8000, 0x8001, 0xc000, 0xfffe, 0xffff };

  for (int i = 0; i < 3000; i++)
    {
      unsigned long d = next () >> (next () & 31);

      if (d == 0)
	d = 1;
      if (check (next (), d) || check (next () >> (next () & 31), d))
	return 1;
    }
  for (int k = 0; k <= 16; k++)
    for (int t = 0; t < 5; t++)
      {
	unsigned long d = (((unsigned long) tops[t] << 16) | 0xffff) >> k, r;

	if (d < 0x10000)
	  continue;
	unsigned long qmax = ref (0xffffffffUL, d, &r);
	unsigned long qs[] = { 1, 2, qmax, qmax - 1, (qmax >> 1) + 1 };

	for (int j = 0; j < 5; j++)
	  {
	    unsigned long m = qs[j] * d;

	    if (check (m, d) || check (m - 1, d) || check (m + d - 1, d))
	      return 2;
	  }
      }
  return 0;
}
