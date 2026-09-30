/* The -fno-exceptions side of init-eh-run.C: the driver must not add the
   ___init_eh reference, so the unwinder glue stays out of the program,
   and the program must still link and run without it.  */
/* { dg-do run } */
/* { dg-options "-fno-exceptions" } */
/* { dg-final { scan-symbol-not "___init_eh" } } */

static int __attribute__ ((noinline))
answer (int v)
{
  return v;
}

int
main (void)
{
  return answer (42) == 42 ? 0 : 1;
}
