int compare_bytes (const char *, const char *)
  __attribute__((noinline, noclone));

int
compare_bytes (const char *p1, const char *p2)
{
  unsigned char a, b;
  for (;;)
    {
      char c = *p1++;
      if (c)
	{
	  if (c == *p2++)
	    continue;
	}
      else
	++p2;
      a = *--p1;
      b = *--p2;
      break;
    }
  return (short) a - b;
}
