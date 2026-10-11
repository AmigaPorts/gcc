/* LTO partitioning promotes statics referenced from another partition to
   hidden visibility.  That is not a user attribute and must not be
   diagnosed on a target without visibility support.  */
/* { dg-lto-do link } */
/* { dg-skip-if "" { ! { m68k-*-amigaos* } } } */
/* { dg-lto-options { { -O2 -flto -flto-partition=max -fno-inline -Werror=attributes } } } */

static int counter;

static int __attribute__ ((noinline))
bump (int x)
{
  counter += x;
  return counter;
}

int __attribute__ ((noinline)) use_a (int x) { return bump (x); }
int __attribute__ ((noinline)) use_b (int x) { return bump (x + 1) + counter; }

extern int other (int);

int
main (void)
{
  return use_a (1) + use_b (2) + other (3) == 0;
}
