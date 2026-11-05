typedef float v2sf __attribute__((vector_size(8)));
typedef union { v2sf v; unsigned bits[2]; } pair;
extern v2sf perm_00 (v2sf, v2sf);
extern v2sf perm_01 (v2sf, v2sf);
extern v2sf perm_02 (v2sf, v2sf);
extern v2sf perm_03 (v2sf, v2sf);
extern v2sf perm_10 (v2sf, v2sf);
extern v2sf perm_11 (v2sf, v2sf);
extern v2sf perm_12 (v2sf, v2sf);
extern v2sf perm_13 (v2sf, v2sf);
extern v2sf perm_20 (v2sf, v2sf);
extern v2sf perm_21 (v2sf, v2sf);
extern v2sf perm_22 (v2sf, v2sf);
extern v2sf perm_23 (v2sf, v2sf);
extern v2sf perm_30 (v2sf, v2sf);
extern v2sf perm_31 (v2sf, v2sf);
extern v2sf perm_32 (v2sf, v2sf);
extern v2sf perm_33 (v2sf, v2sf);
extern v2sf construct (float, float), splat (float);
extern float lane0 (v2sf), lane1 (v2sf), sum (v2sf), dot (v2sf, v2sf);
int test_main (void)
{
  static const unsigned bits[][4] = {
    { 0x3f800000, 0xc0200000, 0x40e00000, 0x41880000 },
    { 0x00000000, 0x80000000, 0x7fc12345, 0xffc54321 }
  };
  for (unsigned k = 0; k < 2; ++k)
    {
      pair a, b, c;
      a.bits[0] = bits[k][0]; a.bits[1] = bits[k][1];
      b.bits[0] = bits[k][2]; b.bits[1] = bits[k][3];
      c.v = perm_00 (a.v, b.v);
      if (c.bits[0] != bits[k][0] || c.bits[1] != bits[k][0])
	return 1;
      c.v = perm_01 (a.v, b.v);
      if (c.bits[0] != bits[k][0] || c.bits[1] != bits[k][1])
	return 2;
      c.v = perm_02 (a.v, b.v);
      if (c.bits[0] != bits[k][0] || c.bits[1] != bits[k][2])
	return 3;
      c.v = perm_03 (a.v, b.v);
      if (c.bits[0] != bits[k][0] || c.bits[1] != bits[k][3])
	return 4;
      c.v = perm_10 (a.v, b.v);
      if (c.bits[0] != bits[k][1] || c.bits[1] != bits[k][0])
	return 5;
      c.v = perm_11 (a.v, b.v);
      if (c.bits[0] != bits[k][1] || c.bits[1] != bits[k][1])
	return 6;
      c.v = perm_12 (a.v, b.v);
      if (c.bits[0] != bits[k][1] || c.bits[1] != bits[k][2])
	return 7;
      c.v = perm_13 (a.v, b.v);
      if (c.bits[0] != bits[k][1] || c.bits[1] != bits[k][3])
	return 8;
      c.v = perm_20 (a.v, b.v);
      if (c.bits[0] != bits[k][2] || c.bits[1] != bits[k][0])
	return 9;
      c.v = perm_21 (a.v, b.v);
      if (c.bits[0] != bits[k][2] || c.bits[1] != bits[k][1])
	return 10;
      c.v = perm_22 (a.v, b.v);
      if (c.bits[0] != bits[k][2] || c.bits[1] != bits[k][2])
	return 11;
      c.v = perm_23 (a.v, b.v);
      if (c.bits[0] != bits[k][2] || c.bits[1] != bits[k][3])
	return 12;
      c.v = perm_30 (a.v, b.v);
      if (c.bits[0] != bits[k][3] || c.bits[1] != bits[k][0])
	return 13;
      c.v = perm_31 (a.v, b.v);
      if (c.bits[0] != bits[k][3] || c.bits[1] != bits[k][1])
	return 14;
      c.v = perm_32 (a.v, b.v);
      if (c.bits[0] != bits[k][3] || c.bits[1] != bits[k][2])
	return 15;
      c.v = perm_33 (a.v, b.v);
      if (c.bits[0] != bits[k][3] || c.bits[1] != bits[k][3])
	return 16;
      c.v = construct (a.v[0], b.v[1]);
      if (c.bits[0] != bits[k][0] || c.bits[1] != bits[k][3]) return 17;
      union { float f; unsigned u; } scalar;
      scalar.f = lane0 (a.v);
      if (scalar.u != bits[k][0]) return 19;
      scalar.f = lane1 (a.v);
      if (scalar.u != bits[k][1]) return 20;
      c.v = splat (a.v[1]);
      if (c.bits[0] != bits[k][1] || c.bits[1] != bits[k][1]) return 18;
    }
  if (sum ((v2sf) { 2.0f, 3.0f }) != 5.0f) return 21;
  if (dot ((v2sf) { 2.0f, 3.0f }, (v2sf) { -4.0f, 5.0f }) != 7.0f)
    return 22;
  return 0;
}
