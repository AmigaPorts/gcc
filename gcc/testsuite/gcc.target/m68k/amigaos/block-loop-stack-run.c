/* { dg-do run } */
/* Block loops after a call whose argument is still on the stack, and block
   copies while the arguments of a call are being pushed.  A lost or misplaced
   stack adjustment fails here or crashes on return.
   The AmigaOS test driver supplies the optimization/allocator/CPU matrix. */
#include <stdlib.h>

#define NOINLINE __attribute__ ((noinline, noclone))

static volatile unsigned calls;

NOINLINE void
use (void *p)
{
  calls += p != 0;
}

NOINLINE void
check_filled (const unsigned char *p, unsigned n, unsigned char v)
{
  for (unsigned i = 0; i < n; ++i)
    if (p[i] != v)
      abort ();
}

NOINLINE void
check_copied (const unsigned char *p, unsigned n)
{
  for (unsigned i = 0; i < n; ++i)
    if (p[i] != (unsigned char) (i * 37 + 19))
      abort ();
}

#define SET(N, V)							\
  NOINLINE void set##N (void *p)					\
  {									\
    unsigned char b[N + 2];						\
    b[N] = 0xa5;							\
    b[N + 1] = 0x5a;							\
    use (p);								\
    __builtin_memset (b, V, N);						\
    check_filled (b, N, V);						\
    if (b[N] != 0xa5 || b[N + 1] != 0x5a)				\
      abort ();								\
  }

#define COPY(N)								\
  NOINLINE void copy##N (void *p, const void *q)			\
  {									\
    unsigned char b[N + 2];						\
    b[N] = 0xa5;							\
    b[N + 1] = 0x5a;							\
    use (p);								\
    __builtin_memcpy (b, q, N);						\
    check_copied (b, N);						\
    if (b[N] != 0xa5 || b[N + 1] != 0x5a)				\
      abort ();								\
  }

#define PASS(N)								\
  struct s##N { unsigned char c[N]; };					\
  NOINLINE int take##N (int x, struct s##N s, int y)			\
  {									\
    check_copied (s.c, N);						\
    return x * 10 + y;							\
  }									\
  NOINLINE int pass##N (void *p, const struct s##N *q)			\
  {									\
    use (p);								\
    return take##N (1, *q, 2);						\
  }									\
  NOINLINE int nest##N (const struct s##N *q)				\
  {									\
    return take##N (take##N (3, *q, 4), *q, take##N (5, *q, 6));	\
  }

SET (16, 0) SET (36, 0x55) SET (60, 0) SET (100, 0) SET (128, 0x55)
SET (300, 0)
COPY (16) COPY (36) COPY (60) COPY (100) COPY (129) COPY (300)
PASS (36) PASS (100) PASS (129) PASS (130) PASS (300) PASS (1000)

static unsigned char src[1000] __attribute__ ((aligned (4)));

int
main (void)
{
  for (unsigned i = 0; i < sizeof src; ++i)
    src[i] = i * 37 + 19;

  set16 (src); set36 (src); set60 (src); set100 (src);
  set128 (src); set300 (src);
  copy16 (src, src); copy36 (src, src); copy60 (src, src);
  copy100 (src, src); copy129 (src, src); copy300 (src, src);

#define RUN(N)								\
  if (pass##N (src, (const struct s##N *) src) != 12			\
      || nest##N ((const struct s##N *) src) != 34 * 10 + 56)		\
    abort ();
  RUN (36) RUN (100) RUN (129) RUN (130) RUN (300) RUN (1000)

  return 0;
}
