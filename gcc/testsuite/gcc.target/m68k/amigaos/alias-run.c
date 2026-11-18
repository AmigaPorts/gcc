/* { dg-do run } */
/* Aliases of functions and data: global, weak, static and #pragma weak.  */

__attribute__ ((noipa)) int foo (int x)
{
  return x + 1;
}

int bar (int) __attribute__ ((alias ("foo")));
int wbar (int) __attribute__ ((weak, alias ("foo")));
static int sbar (int) __attribute__ ((alias ("foo")));

#pragma weak pw = foo
int pw (int);

int d[2] = { 1, 2 };
extern int e[2] __attribute__ ((alias ("d")));

int (*volatile p) (int) = bar;
int *volatile q = &e[1];

int main (void)
{
  if (bar (1) + wbar (1) + sbar (1) + pw (1) != 8)
    return 1;
  if (p != foo || q != &d[1] || *q != 2)
    return 2;
  e[0] = 5;
  return d[0] != 5;
}
