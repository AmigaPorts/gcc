/* Code that runs before a shrink-wrapped prologue reads stack arguments
   at their offsets on function entry.  */
/* { dg-do run } */
/* { dg-options "-O2 -fdump-rtl-pro_and_epilogue" } */

unsigned flag;

__attribute__ ((noipa)) void
mix (unsigned *a, unsigned *b, unsigned *c, unsigned *d)
{
  *a += *b ^ *c;
  *d += *a;
}

__attribute__ ((noipa)) unsigned
f (unsigned *p, unsigned x, unsigned y, unsigned z)
{
  if (flag)
    return y - z;
  unsigned a = p[0], b = p[1], c = p[2], d = p[3];
  mix (&a, &b, &c, &d);
  mix (&d, &c, &b, &a);
  return a + b + c + d + x + y + z;
}

int
main (void)
{
  unsigned v[4] = { 1, 2, 3, 4 };
  flag = 1;
  if (f (v, 10, 50, 8) != 42)
    __builtin_abort ();
  flag = 0;
  if (f (v, 10, 50, 8) != 89)
    __builtin_abort ();
  return 0;
}

/* { dg-final { scan-rtl-dump "Performing shrink-wrapping" "pro_and_epilogue" } } */
