/* In the 68881 extended format a stored exponent of 0 means 2^-16383,
   with the integer bit stored as usual; x87 bumps it to 2^-16382.  The
   soft-float long double code used the x87 reading, so results in or
   below [2^-16383, 2^-16382) came out twice or half as large.  */
/* { dg-do run } */
/* { dg-skip-if "long double is in hardware" { *-*-* } { "-m68881" "-mhard-float" } { "" } } */

struct xf_case { unsigned a[3], b[3], r[3]; char op; };

static const struct xf_case cases[] = {
  /* 2^-16383 * 1, 2^-16382 * 0.5, 2^-16383 + 2^-16383, 2^-16383 / 0.5 */
  { { 0x00000000, 0x80000000, 0 }, { 0x3fff0000, 0x80000000, 0 },
    { 0x00000000, 0x80000000, 0 }, '*' },
  { { 0x00010000, 0x80000000, 0 }, { 0x3ffe0000, 0x80000000, 0 },
    { 0x00000000, 0x80000000, 0 }, '*' },
  { { 0x00000000, 0x80000000, 0 }, { 0x00000000, 0x80000000, 0 },
    { 0x00010000, 0x80000000, 0 }, '+' },
  { { 0x00000000, 0x80000000, 0 }, { 0x3ffe0000, 0x80000000, 0 },
    { 0x00010000, 0x80000000, 0 }, '/' },
  /* Subnormal operands.  */
  { { 0x80370000, 0x8f4d3e27, 0xdda14800 }, { 0x80000000, 0x00000000, 0x19b98d20 },
    { 0x40580000, 0xb24211f9, 0x0ff7c2d4 }, '/' },
  { { 0x80000000, 0xf2518d43, 0x5ecb7348 }, { 0x40160000, 0xdd0c8b94, 0x07bfc096 },
    { 0x80180000, 0xd13c46e4, 0x2e2d0f7a }, '*' },
  { { 0x00000000, 0x78aa8105, 0x735dc326 }, { 0x40190000, 0xa0301309, 0x61eeac37 },
    { 0x001a0000, 0x97027329, 0x59b68dc2 }, '*' },
};

int
main (void)
{
  for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++)
    {
      volatile long double a, b;
      long double r;
      __builtin_memcpy ((void *) &a, cases[i].a, 12);
      __builtin_memcpy ((void *) &b, cases[i].b, 12);
      switch (cases[i].op)
	{
	case '+': r = a + b; break;
	case '*': r = a * b; break;
	default: r = a / b; break;
	}
      if (__builtin_memcmp (&r, cases[i].r, 12))
	__builtin_abort ();
    }
  return 0;
}
