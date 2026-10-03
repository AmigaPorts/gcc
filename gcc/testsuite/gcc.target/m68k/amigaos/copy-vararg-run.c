/* { dg-do run } */
/* Reduced from gcc.c-torture/execute/va-arg-22.c.  Three struct va_args
   in a row: the block copy expander emits post-increment addressing
   before the auto_inc_dec pass, and cse used to keep treating the
   incremented pointer as equal to the struct's address.
   The AmigaOS test driver supplies the optimization/allocator/CPU matrix. */
#include <stdarg.h>
#include <stdlib.h>

void bar (int n, int c)
{
  static int lastn = -1, lastc = -1;

  if (lastn != n)
    {
      if (lastc != lastn)
	abort ();
      lastc = 0;
      lastn = n;
    }

  if (c != (char) (lastc ^ (n << 3)))
    abort ();
  lastc++;
}

#define D(N) typedef struct { char x[N]; } A##N;
D(2) D(12) D(13)
#undef D

void foo (int size, ...)
{
#define D(N) A##N a##N;
D(2) D(12) D(13)
#undef D
  va_list ap;
  int i;

  if (size != 3)
    abort ();
  va_start (ap, size);
#define D(N)					\
  a##N = va_arg (ap, typeof (a##N));		\
  for (i = 0; i < N; i++)			\
    bar (N, a##N.x[i]);
D(2) D(12) D(13)
#undef D
  va_end (ap);
}

int main (void)
{
#define D(N) A##N a##N;
D(2) D(12) D(13)
#undef D
  int i;

#define D(N)					\
  for (i = 0; i < N; i++)			\
    a##N.x[i] = i ^ (N << 3);
D(2) D(12) D(13)
#undef D

  foo (3
#define D(N) , a##N
D(2) D(12) D(13)
#undef D
      );
  exit (0);
}
