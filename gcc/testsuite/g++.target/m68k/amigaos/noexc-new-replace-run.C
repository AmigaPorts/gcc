/* A program may replace one form of operator new and keep the default
   for the others: libnew_noexc.a has one archive member per form, so this
   links, and the default operator new[] routes through the replacement.  */
/* { dg-do run } */
/* { dg-options "-fno-exceptions -O0" } */

#include <new>
#include <stdlib.h>

static int hits;

void *
operator new (size_t n)
{
  hits++;
  return malloc (n ? n : 1);
}

void
operator delete (void *p) noexcept
{
  free (p);
}

void
operator delete (void *p, size_t) noexcept
{
  free (p);
}

int
main (void)
{
  int *v = new int[4] ();
  int *s = new int ();
  int *n = new (std::nothrow) int ();
  int r = v[1] + *s + *n;
  delete[] v;
  delete s;
  delete n;
  return hits == 2 ? r : 1;
}
