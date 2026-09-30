// PR c++/102071
// { dg-do run  { target { { c++17 } && { ! default_packed } } } }
// { dg-additional-options -faligned-new=2 }
// alignof (X) is 2 on m68k, not above the -faligned-new=2 threshold, so the
// plain operator new is the right one and nalign stays 0.
// { dg-skip-if "alignof (X) is 2" { m68k*-*-* } { "*" } { "-malign-int" } }
// { dg-xfail-run-if "AIX operator new" { powerpc-ibm-aix* } }

#include <new>

int nalign;
void *operator new (std::size_t s, std::align_val_t a)
{
  nalign = (int)a;
  return operator new (s);
}

struct X { ~X(); int c; };

int align = (alignof (X) > alignof (std::size_t)
	     ? alignof (X) : alignof (std::size_t));

int n = 4;

int main()
{
  X *p = new X[n];
  if (nalign != align)
    __builtin_abort ();

  X *p2 = new X;
  if (nalign != alignof (X))
    __builtin_abort ();
}
