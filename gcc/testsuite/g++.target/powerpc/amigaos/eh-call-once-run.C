// An exception thrown through std::call_once unwinds through the gthread
// object that -athread links, whose frames have to be registered too.
// A call that throws leaves the flag unset, so the next one runs.
// { dg-do run }

#include <cstdlib>
#include <mutex>

static std::once_flag flag;
static int calls;

int
main ()
{
  try
    {
      std::call_once (flag, [] { calls++; throw 1; });
      abort ();
    }
  catch (int i)
    {
      if (i != 1)
	abort ();
    }
  std::call_once (flag, [] { calls++; });
  std::call_once (flag, [] { abort (); });
  if (calls != 2)
    abort ();
  return 0;
}
