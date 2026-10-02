/* libnew_noexc.a keeps the std::set_new_handler protocol: on failure every
   form of operator new calls the handler and retries, and gives up (null
   for nothrow, abort otherwise) only without one.  */
/* { dg-do run } */
/* { dg-options "-fno-exceptions -O0" } */
/* { dg-final { scan-symbol-not "__cxa_throw" } } */

#include <new>
#include <stdlib.h>

static int calls;

static void
give_up ()
{
  calls++;
  std::set_new_handler (0);
}

static void
done ()
{
  exit (calls == 2 ? 0 : 1);
}

int
main (void)
{
  volatile size_t huge_v = (size_t) 1 << 30;
  const size_t huge = huge_v;

  std::set_new_handler (give_up);
  if (new (std::nothrow) char[huge] != 0 || calls != 1)
    abort ();

  std::set_new_handler (give_up);
  if (new (std::nothrow) int[huge / sizeof (int)] != 0 || calls != 2)
    abort ();

  /* The plain forms abort when there is no handler, so end from it.  */
  std::set_new_handler (done);
  new char[huge];
  abort ();
}
