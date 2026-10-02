/* Non-throwing operator new for -fno-exceptions programs on AmigaOS.

   libsupc++'s operator new throws std::bad_alloc, which pulls the
   unwinder and the terminate handler into every program that uses
   new, even one built with -fno-exceptions.  g++ links libnew_op.a
   ahead of libstdc++ under -fno-exceptions so these definitions are
   used instead.  libgcc is C, so the functions carry the mangled
   names of operator new (std::size_t) and operator new[] (std::size_t);
   size_t is unsigned long on this target.  */

#include <stdlib.h>
#include <unistd.h>

void *_Znwm (unsigned long);
void *_Znam (unsigned long);

void *
_Znwm (unsigned long size)
{
  static const char msg[] = "operator new: out of memory\n";
  void *p = malloc (size ? size : 1);

  if (p)
    return p;
  write (2, msg, sizeof msg - 1);
  abort ();
}

void *
_Znam (unsigned long size)
{
  return _Znwm (size);
}
