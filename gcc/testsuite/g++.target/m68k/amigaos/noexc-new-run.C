/* With -fno-exceptions the driver links libnew_op.a ahead of libstdc++,
   so operator new cannot throw and the unwinder stays out of the
   program.  */
/* { dg-do run } */
/* { dg-options "-fno-exceptions" } */
/* { dg-final { scan-symbol-not "__cxa_throw" } } */

struct A
{
  int x;
};

int
main (void)
{
  A *a = new A ();
  int *v = new int[4];
  int r = a->x + v[0] * 0;
  delete[] v;
  delete a;
  return r;
}
