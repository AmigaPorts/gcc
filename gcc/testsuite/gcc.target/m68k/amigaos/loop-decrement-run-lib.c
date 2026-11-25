extern int values[10];
extern int count, sum;

void
consume (int *p)
{
  if (count >= 10 || p != values + count)
    __builtin_abort ();
  sum += *p;
  ++count;
}
