/* { dg-do run } */
/* dbra loop counts at the edges: 0, 1, the 16-bit sign and borrow
   boundaries, and 32-bit counts with any high word.  The 32-bit loops
   that would run for hours leave early on their own condition.  The
   AmigaOS test driver supplies the optimization and CPU matrix.  */

#define NOINLINE __attribute__ ((noipa))

static volatile unsigned long cnt;

NOINLINE void
up (unsigned long n)
{
  for (unsigned long i = 0; i < n; i++)
    cnt++;
}

NOINLINE void
up_int (int n)
{
  for (int i = 0; i < n; i++)
    cnt++;
}

NOINLINE void
down (unsigned long n)
{
  while (n--)
    cnt++;
}

NOINLINE void
up_us (unsigned short n)
{
  for (unsigned short i = 0; i < n; i++)
    cnt++;
}

/* 0 runs 65536 times.  */
NOINLINE void
do_us (unsigned short n)
{
  do
    cnt++;
  while (--n);
}

/* A loop that also leaves on a condition: the dbcc form.  */
NOINLINE void
until (unsigned long n, unsigned long stop)
{
  while (n--)
    if (++cnt == stop)
      break;
}

NOINLINE void
until_us (unsigned short n, unsigned long stop)
{
  while (n--)
    if (++cnt == stop)
      break;
}

/* 0 runs 2^32 times.  */
NOINLINE void
do_until (unsigned long n, unsigned long stop)
{
  do
    if (++cnt == stop)
      break;
  while (--n);
}

/* Eight live values push the counter out of the data registers.  */
#define BODY							\
  a += b ^ c; b += c ^ d; c += d ^ e; d += e ^ f;		\
  e += f ^ g; f += g ^ h; g += h ^ a; h += a ^ b;		\
  cnt++
#define LOAD							\
  unsigned long a = p[0], b = p[1], c = p[2], d = p[3];	\
  unsigned long e = p[4], f = p[5], g = p[6], h = p[7]
#define SUM (a + b + c + d + e + f + g + h)

NOINLINE unsigned long
pressure (const unsigned long *p, unsigned long n)
{
  LOAD;
  unsigned long m = n & 0xffff;
  for (unsigned long i = 0; i < m; i++)
    {
      BODY;
    }
  return SUM;
}

NOINLINE unsigned long
pressure_us (const unsigned long *p, unsigned short n)
{
  LOAD;
  for (unsigned short i = 0; i < n; i++)
    {
      BODY;
    }
  return SUM;
}

NOINLINE unsigned long
pressure_40000 (const unsigned long *p)
{
  LOAD;
  for (int i = 0; i < 40000; i++)
    {
      BODY;
    }
  return SUM;
}

static const unsigned long v[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };

static void
check (unsigned long expect)
{
  if (cnt != expect)
    __builtin_abort ();
  cnt = 0;
}

int
main (void)
{
  static const unsigned long counts[] = {
    0, 1, 2, 32767, 32768, 32769, 65534, 65535, 65536, 65537, 0x20001
  };

  for (unsigned k = 0; k < sizeof counts / sizeof counts[0]; k++)
    {
      unsigned long n = counts[k];
      unsigned short s = n;

      up (n); check (n);
      up_int (n); check (n);
      down (n); check (n);
      up_us (s); check (s);
      do_us (s); check (s ? s : 65536);
      until (n, -1); check (n);
      until_us (s, -1); check (s);
      if (n)
	{
	  do_until (n, -1);
	  check (n);
	}
    }

  until (0xffffffff, 70000); check (70000);
  until (0x80000000, 70000); check (70000);
  until (0x80010005, 70000); check (70000);
  until (0xfffe0003, 70000); check (70000);
  do_until (0, 70000); check (70000);
  do_until (0x80010005, 70000); check (70000);

  unsigned long r = pressure_40000 (v); check (40000);
  if (pressure (v, 40000) != r)
    __builtin_abort ();
  check (40000);
  if (pressure_us (v, 40000) != r)
    __builtin_abort ();
  check (40000);
  return 0;
}
