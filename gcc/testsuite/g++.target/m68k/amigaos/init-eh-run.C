/* AmigaOS has no crtbegin.o, so the .eh_frame sections are registered
   with the unwinder by the C library's __init_eh, which the g++ driver
   pulls in with -Wl,-u,___init_eh whenever exceptions are enabled.  If
   that reference is lost, this program links fine and then terminates
   on the first throw, because the unwinder finds no frame info at all.
   Run with the default options: no -fexceptions on the command line,
   which is how every C++ program is built.  The symbol check names the
   cause when the run fails.  */
/* { dg-do run } */
/* { dg-final { scan-symbol "___init_eh" } } */

struct E
{
  int v;
};

static void __attribute__ ((noinline))
thrower (int v)
{
  throw E{v};
}

int
main (void)
{
  try
    {
      thrower (42);
    }
  catch (const E &e)
    {
      return e.v == 42 ? 0 : 1;
    }
  return 2;
}
