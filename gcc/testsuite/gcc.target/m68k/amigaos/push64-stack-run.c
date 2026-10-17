/* Derived from OpenAmigaGCC; Copyright (C) 2026 Dalsin Limited.  */
/* { dg-do run } */
/* { dg-options "-Os -m68040 -fomit-frame-pointer" } */
/* 64-bit libcall arguments pushed from stack slots (-m68040/-m68060 tuning,
   no frame pointer): (s64)m * 4294967296 / det pushes 0 and then m's low
   word; that second push must not read 4 bytes too low.
   The arithmetic is OpenGPU's affine compositing, reduced. */
typedef long long s64;
volatile long M[6] = { 0x18000, 0x6000, 0x30000, -0x4000, 0x14000, 0x20000 };
volatile s64 sample = 0x123456789abcdef0LL;
static const long want[9] = { 41610, -12483, 8322, 49932, 196608, 4866048, -524288, 2588672, -1912834220 };
__attribute__((noinline)) static s64 shift_div(s64 a, long b) {
  return (a >> 32) / b;
}
__attribute__((noinline)) int calc(long *out) {
  long m00 = M[0], m01 = M[1], m02 = M[2], m10 = M[3], m11 = M[4], m12 = M[5];
  s64 det = (s64)m00 * m11 - (s64)m01 * m10, i00, i01, i10, i11;
  if (det == 0) return 1;
  i00 = (s64)m11 * 4294967296LL / det; i01 = -((s64)m01 * 4294967296LL) / det;
  i10 = -((s64)m10 * 4294967296LL) / det; i11 = (s64)m00 * 4294967296LL / det;
  long minx = 0x7FFFFFFFL, maxx = -0x7FFFFFFFL, miny = 0x7FFFFFFFL, maxy = -0x7FFFFFFFL;
  for (int k = 0; k < 4; k++) {
    long u = k & 1 ? 40L << 16 : 0, v = k & 2 ? 30L << 16 : 0;
    long tx = (long)(((s64)m00 * u + (s64)m01 * v) / 65536) + m02;
    long ty = (long)(((s64)m10 * u + (s64)m11 * v) / 65536) + m12;
    if (tx < minx) minx = tx;
    if (tx > maxx) maxx = tx;
    if (ty < miny) miny = ty;
    if (ty > maxy) maxy = ty;
  }
  unsigned long acc = 0;
  for (int y = 0; y < 8; y++) {
    s64 dx = ((s64)3 << 16) + 0x8000 - m02, dy = ((s64)y << 16) + 0x8000 - m12;
    long u = (long)((i00 * dx + i01 * dy) >> 16), v = (long)((i10 * dx + i11 * dy) >> 16);
    acc = acc * 31 + u; acc = acc * 31 + v;
  }
  out[0] = (long)i00; out[1] = (long)i01; out[2] = (long)i10; out[3] = (long)i11;
  out[4] = minx; out[5] = maxx; out[6] = miny; out[7] = maxy; out[8] = acc;
  return 0;
}
int main(void) {
  long o[9]; int bad = 0;
  calc(o);
  for (int i = 0; i < 9; i++) if (o[i] != want[i]) bad++;
  if (shift_div(sample, 3) != 0x6117228LL)
    bad++;
  return bad != 0;
}
