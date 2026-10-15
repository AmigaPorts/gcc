/* A program's calloc may be implemented with malloc and memset.  */
/* { dg-do run } */
/* { dg-options "-O2" } */

#include <stddef.h>

static unsigned char pool[4096];
static size_t used;
static int depth;

__attribute__((noinline, noclone)) void *
malloc (size_t size)
{
  void *result = pool + used;
  used += (size + 7) & ~(size_t) 7;
  return result;
}

void
free (void *ptr)
{
  (void) ptr;
}

void *
calloc (size_t count, size_t size)
{
  if (++depth > 1)
    __builtin_abort ();
  size_t bytes = count * size;
  void *result = malloc (bytes);
  if (result)
    __builtin_memset (result, 0, bytes);
  depth--;
  return result;
}

int
main (void)
{
  for (int i = 0; i < 64; i++)
    pool[i] = 0xaa;
  unsigned char *result = calloc (8, 8);
  if (!result)
    __builtin_abort ();
  for (int i = 0; i < 64; i++)
    if (result[i])
      __builtin_abort ();
  return 0;
}
