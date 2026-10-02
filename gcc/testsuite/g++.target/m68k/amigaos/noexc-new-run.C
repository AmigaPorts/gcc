/* With -fno-exceptions the driver links libnew_noexc.a ahead of libstdc++,
   so no form of operator new can throw and the unwinder stays out of
   the program.  */
/* { dg-do run } */
/* { dg-options "-fno-exceptions" } */
/* { dg-final { scan-symbol-not "__cxa_throw" } } */

#include <new>
#include <stdlib.h>
#include <stdint.h>

struct A
{
  int x;
};

/* Over the default new alignment: uses the align_val_t forms.  */
struct alignas (16) B
{
  int x;
};

int
main (void)
{
  A *a = new A ();
  int *v = new int[4] ();
  int r = a->x + v[0];
  delete[] v;
  delete a;

  B *b = new B ();
  B *bv = new B[3] ();
  if (((uintptr_t) b & 15) || ((uintptr_t) bv & 15))
    abort ();
  r += b->x + bv[2].x;
  delete[] bv;
  delete b;

  /* The nothrow forms return null instead of aborting.  */
  int *n = new (std::nothrow) int[4] ();
  B *bn = new (std::nothrow) B ();
  if (!n || !bn || ((uintptr_t) bn & 15))
    abort ();
  r += n[3] + bn->x;
  delete[] n;
  delete bn;

  volatile size_t huge_v = (size_t) 1 << 30;
  const size_t huge = huge_v;
  if (new (std::nothrow) char[huge] != 0)
    abort ();
  if (new (std::nothrow) B[huge / sizeof (B)] != 0)
    abort ();

  /* Alignment padding must not wrap the size around to a small one.  */
  volatile size_t near_max_v = (size_t) -1 - 8;
  const size_t near_max = near_max_v;
  if (operator new (near_max, std::align_val_t (16), std::nothrow) != 0)
    abort ();
  if (operator new[] (near_max, std::align_val_t (16), std::nothrow) != 0)
    abort ();

  return r;
}
