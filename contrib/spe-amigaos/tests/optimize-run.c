typedef float v2sf __attribute__((vector_size(8)));
extern v2sf vector_add (v2sf, v2sf), vector_sub (v2sf, v2sf);
extern v2sf vector_mul (v2sf, v2sf), vector_div (v2sf, v2sf);
extern v2sf vector_neg (v2sf);
extern void array_add (float *, const float *, const float *, unsigned);
extern void array_add_int (int *, const int *, const int *, unsigned);
extern void array_abs (float *, const float *, unsigned);
extern double double_div (double, double), double_add (double, double);
extern unsigned long long double_bits (double);

int
test_main (void)
{
  v2sf a = { 8.0f, -12.0f }, b = { 2.0f, 4.0f }, v;
  v = vector_add (a, b);
  if (v[0] != 10.0f || v[1] != -8.0f) return 1;
  v = vector_sub (a, b);
  if (v[0] != 6.0f || v[1] != -16.0f) return 2;
  v = vector_mul (a, b);
  if (v[0] != 16.0f || v[1] != -48.0f) return 3;
  v = vector_div (a, b);
  if (v[0] != 4.0f || v[1] != -3.0f) return 4;
  v = vector_neg (a);
  if (v[0] != -8.0f || v[1] != 12.0f) return 5;
  if (double_div (7.5, 2.0) != 3.75) return 6;
  if (double_add (7.5, -2.0) != 5.5) return 7;
  if (double_bits (5.5) != 0x4016000000000000ULL) return 8;
  if (double_bits (-0.0) != 0x8000000000000000ULL) return 9;

  float fa[40] __attribute__((aligned(8)));
  float fb[40] __attribute__((aligned(8)));
  float fc[40] __attribute__((aligned(8)));
  int ia[40] __attribute__((aligned(8)));
  int ib[40] __attribute__((aligned(8)));
  int ic[40] __attribute__((aligned(8)));
  for (unsigned i = 0; i < 40; ++i)
    {
      ia[i] = (int) i - 20;
      ib[i] = (int) i * 2;
      fa[i] = ia[i];
      fb[i] = ib[i];
    }
  /* Exercise empty loops, odd tails, and every combination of naturally
     aligned scalar pointers with/without eight-byte vector alignment.  */
  for (unsigned n = 0; n <= 33; ++n)
    for (unsigned x = 0; x < 2; ++x)
      for (unsigned y = 0; y < 2; ++y)
	for (unsigned z = 0; z < 2; ++z)
	  {
	    for (unsigned i = 0; i < 40; ++i)
	      { fc[i] = 999.0f; ic[i] = 999; }
	    array_add (fc + z, fa + x, fb + y, n);
	    array_add_int (ic + z, ia + x, ib + y, n);
	    for (unsigned i = 0; i < 40; ++i)
	      {
		int expected = 999;
		if (i >= z && i < z + n)
		  expected = ia[i - z + x] + ib[i - z + y];
		if (fc[i] != (float) expected || ic[i] != expected)
		  return 100 + n;
	      }
	    array_abs (fc + z, fa + x, n);
	    for (unsigned i = 0; i < n; ++i)
	      if (fc[i + z] != (float) (ia[i + x] < 0
				       ? -ia[i + x] : ia[i + x]))
		return 200 + n;
	  }
  return 0;
}
