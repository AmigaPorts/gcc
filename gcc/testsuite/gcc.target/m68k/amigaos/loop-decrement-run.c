/* { dg-do run } */
/* { dg-options "-fbaserel -mregparm=4 -msoft-float" } */
/* { dg-additional-sources "loop-decrement-run-lib.c" } */
extern void consume (int *);
int values[10];
int count, sum;

__attribute__ ((noinline, noclone)) void
visit (void)
{
  for (int i = 0; i < 10; ++i)
    consume (&values[i]);
}

int
main (void)
{
  for (int i = 0; i < 10; ++i)
    values[i] = 3 * i + 1;
  visit ();
  if (count != 10 || sum != 145)
    __builtin_abort ();
  return 0;
}
