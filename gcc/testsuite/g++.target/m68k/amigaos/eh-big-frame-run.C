/* An exception thrown out of a function whose frame is too large for a
   16-bit displacement must unwind: the CFI has to describe the frame's
   long stack adjustment.  As g++.dg/torture/pr119610.C, with a frame just
   past 32 KB rather than 512 KB, so that it fits in the test board's
   stack.  */
/* { dg-do run } */
/* { dg-options "-O2 -fomit-frame-pointer" } */

int *ptr;

__attribute__((noipa)) void
foo ()
{
  int c[1024 * 10];
  ptr = c;
  throw 1;
}

int
main ()
{
  try
    {
      foo ();
    }
  catch (int x)
    {
      return x != 1;
    }
  return 1;
}
