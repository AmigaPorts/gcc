// newlib's and clib2's startup code does not register the program's
// .eh_frame with the unwinder, so the driver links crtehbegin.o and
// crtehend.o to do it.  Without them the first throw ends in a Guru
// Meditation.  Unwind through several frames, running a destructor on
// the way, catch by base class and rethrow.
// { dg-do run }

#include <cstdlib>

struct B { int v; };
struct D : B { D (int i) : B{i} {} };

static int destroyed;
struct Guard { ~Guard () { destroyed++; } };

static void __attribute__ ((noinline))
thrower (int v)
{
  throw D (v);
}

static void __attribute__ ((noinline))
middle (int v)
{
  Guard g;
  thrower (v);
}

static void __attribute__ ((noinline))
rethrower (int v)
{
  try
    {
      middle (v);
    }
  catch (const B &b)
    {
      if (b.v != v)
	abort ();
      throw;
    }
}

int
main ()
{
  try
    {
      rethrower (42);
    }
  catch (const D &d)
    {
      if (d.v != 42 || destroyed != 1)
	abort ();
      return 0;
    }
  abort ();
}
