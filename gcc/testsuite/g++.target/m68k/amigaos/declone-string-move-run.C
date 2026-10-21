/* Moving a std::string at -Os must link.  -fdeclone-ctor-dtor, which -Os
   implies, makes the move constructor call a unified C4 constructor that
   libstdc++'s explicit instantiation does not export, so the driver turns
   it off (PR c++/125359).  */
/* { dg-do run } */
/* { dg-options "-Os -std=gnu++20" } */

#include <string>
#include <utility>

std::string g;

__attribute__ ((noinline)) void
take (std::string s)
{
  g = s;
}

int
main ()
{
  std::string a ("long enough to live on the heap");
  take (std::move (a));
  return g.size () != 31;
}
