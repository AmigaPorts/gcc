/* The soft-float long double add and subtract rounded an effective
   subtraction (operands of opposite sign) 1 or 2 ulp away from zero when
   the first operand had the smaller exponent: the bits shifted out of it
   were dropped instead of borrowed from.  */
/* { dg-do run } */
/* { dg-skip-if "long double is in hardware" { *-*-* } { "-m68881" "-mhard-float" } { "" } } */

struct xf_case { unsigned a[3], b[3], r[3]; char op; };

static const struct xf_case cases[] = {
  { { 0x40010000, 0xd3ac94af, 0x0f21ddb6 }, { 0x403e0000, 0xa09f76b5, 0xa170b338 },
    { 0xc03e0000, 0xa09f76b5, 0xa170b331 }, '-' },
  { { 0x3fd10000, 0xfc241d0b, 0xc9d488b1 }, { 0xbff10000, 0xcda6c6fd, 0xbd685167 },
    { 0xbff10000, 0xcda6c6fc, 0xc144345b }, '+' },
  { { 0xc0040000, 0xa6b1cffc, 0x070d7109 }, { 0xc03e0000, 0x9c9011ef, 0x256badf9 },
    { 0x403e0000, 0x9c9011ef, 0x256badcf }, '-' },
  { { 0xbffe0000, 0xc265bb31, 0x53740902 }, { 0xc03e0000, 0xe8f6e0bd, 0x0f977044 },
    { 0x403e0000, 0xe8f6e0bd, 0x0f977043 }, '-' },
  { { 0xbfbb0000, 0xf0c1dca1, 0x756b7289 }, { 0x3fbc0000, 0x9fb9af50, 0x84768b8c },
    { 0x3fba0000, 0x9d6303ff, 0x2703491e }, '+' },
  { { 0x40030000, 0x95a0a8ae, 0x3b996870 }, { 0xc03e0000, 0xb74b589b, 0xe48e9e02 },
    { 0xc03e0000, 0xb74b589b, 0xe48e9def }, '+' },
  /* A normal number minus a subnormal one.  */
  { { 0x00180000, 0xe18d4a00, 0x00000000 }, { 0x00000000, 0x00000000, 0x40000000 },
    { 0x00180000, 0xe18d49ff, 0xffffffc0 }, '-' },
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
      r = cases[i].op == '+' ? a + b : a - b;
      if (__builtin_memcmp (&r, cases[i].r, 12))
	__builtin_abort ();
    }
  return 0;
}
