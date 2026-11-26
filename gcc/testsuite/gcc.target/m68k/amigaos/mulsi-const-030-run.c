/* { dg-do run } */

/* Exercise small scales, values around powers of two, and constants that
   require more than MOVEQ, with inputs that wrap the unsigned product.  */
#define FACTORS(M) \
  M(0) M(1) M(2) M(3) M(5) M(7) M(9) M(10) M(13) M(17) \
  M(31) M(33) M(39) M(51) M(52) M(53) M(63) M(64) M(65) \
  M(104) M(127) M(128) M(129) M(134) M(138) M(140) M(142) \
  M(146) M(148) M(150) M(208) M(255) M(256) M(257) \
  M(32767) M(32769) M(65535) M(65537) M(0x12345678U) \
  M(0x80000001U) M(0xffffffffU)

#define DEFINE_MULT(N) \
  __attribute__ ((noinline, noclone)) unsigned int \
  mul_##N (unsigned int x) { return x * (N); }
FACTORS(DEFINE_MULT)

#define ENTRY(N) { N, mul_##N },
static const struct {
  unsigned int factor;
  unsigned int (*fn) (unsigned int);
} cases[] = { FACTORS(ENTRY) };

int
main (void)
{
  static const unsigned int inputs[] = {
    0, 1, 2, 32767, 32768, 65535, 65536,
    0x12345678U, 0x40000000U, 0x7fffffffU,
    0x80000000U, 0x80000001U, 0xffffffffU
  };
  for (unsigned int i = 0; i < sizeof cases / sizeof cases[0]; ++i)
    for (unsigned int j = 0; j < sizeof inputs / sizeof inputs[0]; ++j)
      {
	unsigned int factor = cases[i].factor;
	unsigned int x = inputs[j];
	unsigned int expected = 0;
	while (factor)
	  {
	    if (factor & 1)
	      expected += x;
	    factor >>= 1;
	    x <<= 1;
	    /* Keep the reference as a shift/add loop.  */
	    __asm__ ("" : "+r" (expected), "+r" (factor), "+r" (x));
	  }
	if (cases[i].fn (inputs[j]) != expected)
	  __builtin_abort ();
      }
  return 0;
}
