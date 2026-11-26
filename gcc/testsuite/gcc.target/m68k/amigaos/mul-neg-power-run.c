/* { dg-do run } */

/* Unsigned arithmetic checks wrapping products, including the top bit.
   The same cost shortcut is used for word, long and double-word modes.  */
#define FACTORS32(M) \
  M(1) M(2) M(3) M(4) M(5) M(6) M(7) M(8) M(9) M(10) \
  M(11) M(12) M(13) M(14) M(15) M(16) M(17) M(18) M(19) M(20) \
  M(21) M(22) M(23) M(24) M(25) M(26) M(27) M(28) M(29) M(30) M(31)
#define FACTORS64(M) \
  M(1) M(2) M(3) M(7) M(8) M(9) M(16) M(30) M(31) M(32) M(62) M(63)

#define DEFINE32(N) \
  __attribute__ ((noinline, noclone)) unsigned int \
  negative32_##N (unsigned int x) { return x * -(1U << (N)); }
#define DEFINE64(N) \
  __attribute__ ((noinline, noclone)) unsigned long long \
  negative64_##N (unsigned long long x) { return x * -(1ULL << (N)); }
FACTORS32(DEFINE32)
FACTORS64(DEFINE64)

#define ENTRY32(N) { -(1U << (N)), negative32_##N },
#define ENTRY64(N) { -(1ULL << (N)), negative64_##N },
static const struct {
  unsigned int factor;
  unsigned int (*fn) (unsigned int);
} cases32[] = { FACTORS32(ENTRY32) };
static const struct {
  unsigned long long factor;
  unsigned long long (*fn) (unsigned long long);
} cases64[] = { FACTORS64(ENTRY64) };

__attribute__ ((noinline, noclone)) int
negative_word (short x)
{
  return x * -128;
}

__attribute__ ((noinline, noclone)) int
negative_long (int x)
{
  return x * -65536;
}

int
main (void)
{
  static const unsigned long long inputs[] = {
    0, 1, 2, 32767, 32768, 65535, 65536, 0x12345678ULL,
    0x7fffffffULL, 0x80000000ULL, 0xffffffffULL,
    0x100000000ULL, 0x123456789abcdef0ULL,
    0x7fffffffffffffffULL, 0x8000000000000000ULL,
    0xffffffffffffffffULL
  };
  for (unsigned int i = 0; i < sizeof cases32 / sizeof cases32[0]; ++i)
    for (unsigned int j = 0; j < sizeof inputs / sizeof inputs[0]; ++j)
      {
	unsigned int factor = cases32[i].factor;
	unsigned int x = inputs[j];
	unsigned int expected = 0;
	while (factor)
	  {
	    if (factor & 1)
	      expected += x;
	    factor >>= 1;
	    x <<= 1;
	    __asm__ ("" : "+r" (expected), "+r" (factor), "+r" (x));
	  }
	if (cases32[i].fn (inputs[j]) != expected)
	  __builtin_abort ();
      }
  for (unsigned int i = 0; i < sizeof cases64 / sizeof cases64[0]; ++i)
    for (unsigned int j = 0; j < sizeof inputs / sizeof inputs[0]; ++j)
      {
	unsigned long long factor = cases64[i].factor;
	unsigned long long x = inputs[j];
	unsigned long long expected = 0;
	while (factor)
	  {
	    if (factor & 1)
	      expected += x;
	    factor >>= 1;
	    x <<= 1;
	    __asm__ ("" : "+r" (expected), "+r" (factor), "+r" (x));
	  }
	if (cases64[i].fn (inputs[j]) != expected)
	  __builtin_abort ();
      }
  /* Keep signed products inside the representable range.  */
  for (int x = -32767; x <= 32767; x += 32767)
    {
      volatile int factor_word = -128;
      volatile int factor_long = -65536;
      if (negative_word (x) != x * factor_word
	  || negative_long (x) != x * factor_long)
	__builtin_abort ();
    }
  return 0;
}
