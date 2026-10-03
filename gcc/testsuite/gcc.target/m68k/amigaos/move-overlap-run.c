/* { dg-do run } */
/* Overlapping memmove through pointers: the movmemsi expander cannot
   prove the copy direction here and must leave it to the library.
   The AmigaOS test driver supplies the optimization/allocator/CPU matrix. */
#include <stdlib.h>

struct s { char b[40]; };

__attribute__((noinline,noclone)) void up_bytes(char *p)
{ __builtin_memmove(p + 1, p, 16); }

__attribute__((noinline,noclone)) void down_bytes(char *p)
{ __builtin_memmove(p, p + 1, 16); }

__attribute__((noinline,noclone)) void up_longs(long *p)
{ __builtin_memmove(p + 1, p, 32); }

__attribute__((noinline,noclone)) void up_in_struct(struct s *s)
{ __builtin_memmove(s->b + 1, s->b, 16); }

int main(void)
{
  char a[20];
  long l[9];
  struct s st;
  int i;

  for (i = 0; i < 20; i++) a[i] = 'a' + i;
  up_bytes(a);
  for (i = 1; i < 17; i++) if (a[i] != 'a' + i - 1) abort();

  for (i = 0; i < 20; i++) a[i] = 'a' + i;
  down_bytes(a);
  for (i = 0; i < 16; i++) if (a[i] != 'a' + i + 1) abort();

  for (i = 0; i < 9; i++) l[i] = i + 1;
  up_longs(l);
  for (i = 1; i < 9; i++) if (l[i] != i) abort();

  for (i = 0; i < 40; i++) st.b[i] = 'a' + i;
  up_in_struct(&st);
  for (i = 1; i < 17; i++) if (st.b[i] != 'a' + i - 1) abort();

  return 0;
}
