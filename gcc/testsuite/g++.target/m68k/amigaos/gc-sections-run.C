// --gc-sections must keep the static constructor of gc-sections-aux.cc, an
// unreferenced file: its .list_ entry makes the startup code call it.  The
// exception checks that the unwinder still finds its tables.
// { dg-do run }
// { dg-options "-Os -Wl,--gc-sections" }
// { dg-additional-sources "gc-sections-aux.cc" }

#include <cstdio>
#include <stdexcept>

int flag;

static void
thrower (int i)
{
  if (i > 0)
    throw std::runtime_error ("caught");
}

int
main (int argc, char **)
{
  try
    {
      thrower (argc);
    }
  catch (const std::exception &e)
    {
      std::printf ("%s %d\n", e.what (), flag);
      return flag == 42 ? 0 : 1;
    }
  return 2;
}

// { dg-output "caught 42" }
