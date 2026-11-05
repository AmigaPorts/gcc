/* { dg-do run } */
/* { dg-options "-O2 -m68020" } */

/* Check unsigned-byte ordering, termination and independently aligned
   pointers.  Only the sign of strcmp's result is specified.  */

#include "byte-compare.h"

static unsigned char a[80], b[80];
static unsigned long cases;

static int
check (const unsigned char *p, const unsigned char *q, int expected)
{
  int got = compare_bytes ((const char *) p, (const char *) q);

  ++cases;
  if ((got > 0) - (got < 0) != (expected > 0) - (expected < 0))
    {
      __builtin_abort ();
      return 1;
    }
  return 0;
}

int
main (void)
{
  int x, y, i, j, n, k;

  /* Every pair includes zero and bytes above CHAR_MAX.  */
  for (x = 0; x < 256; ++x)
    for (y = 0; y < 256; ++y)
      {
	a[0] = x;
	b[0] = y;
	a[1] = b[1] = 0;
	if (check (a, b, x - y))
	  return 1;
      }

  for (i = 0; i < 4; ++i)
    for (j = 0; j < 4; ++j)
      for (n = 0; n <= 64; ++n)
	{
	  unsigned char *p = a + i, *q = b + j;

	  for (k = 0; k < n; ++k)
	    p[k] = q[k] = (k & 1) ? 0x80 : 0xff;
	  p[n] = q[n] = 0;
	  if (check (p, q, 0) || check (p, p, 0))
	    return 1;

	  p[n] = 0x80;
	  p[n + 1] = 0;
	  if (check (p, q, 1) || check (q, p, -1))
	    return 1;

	  q[n] = 0xff;
	  q[n + 1] = 0;
	  if (check (p, q, -1) || check (q, p, 1))
	    return 1;
	}

  if (cases != 71776)
    __builtin_abort ();
  return 0;
}
