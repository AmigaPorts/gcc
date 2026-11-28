/* A state update used on occasional callbacks. */
unsigned
update (unsigned *p, unsigned x)
{
  unsigned a = p[0], b = p[1], c = p[2], d = p[3];
  unsigned e = p[4], f = p[5], g = p[6], h = p[7];
  for (unsigned i = 0; i < 3; ++i)
    {
      a += (b ^ h) + x; b ^= (c + a) >> 3;
      c += (d ^ b) + x; d ^= (e + c) >> 5;
      e += (f ^ d) + x; f ^= (g + e) >> 7;
      g += (h ^ f) + x; h ^= (a + g) >> 9;
    }
  p[0] = a; p[1] = b; p[2] = c; p[3] = d;
  p[4] = e; p[5] = f; p[6] = g; p[7] = h;
  return a ^ c ^ e ^ g;
}
