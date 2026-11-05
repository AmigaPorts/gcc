/* Freestanding test support. Volatile stores prevent loop-to-memset folding. */
void *memset(void *dst, int value, __SIZE_TYPE__ size)
{
  volatile unsigned char *p = dst;
  for (__SIZE_TYPE__ i = 0; i < size; ++i)
    p[i] = value;
  return dst;
}
