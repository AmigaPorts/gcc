/* { dg-do run } */
/* { dg-options "-fbaserel -mregparm=4 -msoft-float" } */
int values[4] = { 3, 9, 27, 81 };

__attribute__ ((noinline, noclone)) int *
address (void)
{
  return values;
}

__attribute__ ((noinline, noclone)) void
consume (int *p)
{
  if (p != values || p[0] != 3 || p[3] != 81)
    __builtin_abort ();
  p[1] = 12;
}

int
main (void)
{
  int *p = address ();
  if (p != values || p[0] != 3 || p[3] != 81)
    __builtin_abort ();
  consume (values);
  if (p[1] != 12)
    __builtin_abort ();
  return 0;
}
