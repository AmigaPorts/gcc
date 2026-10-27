// crtehbegin.o registers the .eh_frame in a priority-0 constructor, which
// has to run before any other.  Throw and catch in constructors of
// priority 101 and 200 and of default priority, and check that they ran
// in that order.
// { dg-do run }

#include <cstdlib>

static int order[3], n;

static void __attribute__ ((noinline))
thrower (int v)
{
  throw v;
}

static void
caught (int v)
{
  try
    {
      thrower (v);
    }
  catch (int i)
    {
      if (n < 3)
	order[n] = i;
      n++;
    }
}

static void __attribute__ ((constructor (101))) c101 () { caught (101); }
static void __attribute__ ((constructor (200))) c200 () { caught (200); }
static struct S { S () { caught (1); } } s;

int
main ()
{
  if (n != 3 || order[0] != 101 || order[1] != 200 || order[2] != 1)
    abort ();
  return 0;
}
