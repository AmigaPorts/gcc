/* Constructor and destructor priorities.  The entries go to
   .list___CTOR_LIST__.NNNNN and .list___DTOR_LIST__.NNNNN sections, which
   the linker script sorts by name behind the default lists; the startup
   code walks the constructors backwards and the destructors forwards.
   The functions are defined out of order so that link order alone
   cannot produce the right sequence.  A destructor that runs out of
   turn aborts, which the harness sees as a failed run.  */
/* { dg-do run } */

static int seq;

static void __attribute__ ((constructor (700)))
c700 (void)
{
  if (seq++ != 2)
    __builtin_abort ();
}

static void __attribute__ ((constructor))
cdef (void)
{
  if (seq++ != 3)
    __builtin_abort ();
}

static void __attribute__ ((constructor (500)))
c500 (void)
{
  if (seq++ != 0)
    __builtin_abort ();
}

static void __attribute__ ((constructor (600)))
c600 (void)
{
  if (seq++ != 1)
    __builtin_abort ();
}

static void __attribute__ ((destructor (500)))
d500 (void)
{
  if (seq++ != 7)
    __builtin_abort ();
}

static void __attribute__ ((destructor))
ddef (void)
{
  if (seq++ != 4)
    __builtin_abort ();
}

static void __attribute__ ((destructor (700)))
d700 (void)
{
  if (seq++ != 5)
    __builtin_abort ();
}

static void __attribute__ ((destructor (600)))
d600 (void)
{
  if (seq++ != 6)
    __builtin_abort ();
}

int
main (void)
{
  return seq == 4 ? 0 : 1;
}
