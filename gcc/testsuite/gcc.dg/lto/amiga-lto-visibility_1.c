static int __attribute__ ((noinline))
helper (int x)
{
  return x * 3;
}

int __attribute__ ((noinline))
other (int x)
{
  return helper (x) + helper (x + 1);
}
