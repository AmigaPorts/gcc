/* <stdbit.h>, inline on gcc's builtins, with a runtime that has none of its
   functions: each typed function and its type-generic form against a bit
   by bit count, for every unsigned char and unsigned short, and for 0, all
   ones and each single bit with its neighbours in the wider types.  Each
   check prints what failed; the exit code is the number of failures.
   stdbit-newlib-run.c runs it with newlib.  */

/* { dg-do run } */

#include <limits.h>
#include <stdbit.h>
#include <stdio.h>

static int failures;

typedef unsigned long long u64;

static unsigned width;

static int bit(u64 x, unsigned i)
{
  return (x >> i) & 1;
}

static unsigned ref_leading(u64 x, int b)
{
  unsigned n = 0;
  while (n < width && bit(x, width - 1 - n) == b)
    n++;
  return n;
}

static unsigned ref_trailing(u64 x, int b)
{
  unsigned n = 0;
  while (n < width && bit(x, n) == b)
    n++;
  return n;
}

static unsigned ref_count(u64 x, int b)
{
  unsigned n = 0;
  for (unsigned i = 0; i < width; i++)
    n += bit(x, i) == b;
  return n;
}

static unsigned ref_first_leading(u64 x, int b)
{
  unsigned n = ref_leading(x, !b);
  return n == width ? 0 : n + 1;
}

static unsigned ref_first_trailing(u64 x, int b)
{
  unsigned n = ref_trailing(x, !b);
  return n == width ? 0 : n + 1;
}

static unsigned ref_bit_width(u64 x)
{
  return width - ref_leading(x, 0);
}

static u64 ref_bit_floor(u64 x)
{
  return x == 0 ? 0 : 1ULL << (ref_bit_width(x) - 1);
}

static u64 ref_bit_ceil(u64 x)
{
  /* 0 when the result doesn't fit, which C23 leaves unspecified */
  if (x <= 1)
    return 1;
  unsigned w = ref_bit_width(x - 1);
  return w >= width ? 0 : 1ULL << w;
}

static void check(u64 got, u64 want, const char *name, u64 x)
{
  if (got != want)
    {
      printf("FAIL: %s(%#llx) = %#llx, not %#llx\n", name, x, got, want);
      failures++;
    }
}

/* Each typed function, then the type-generic form on a value of its type;
   bit_ceil only where the result fits.  */
#define CHECK_TYPE(T, suffix, x)					\
  do {									\
    T v = (T)(x);							\
    width = sizeof(T) * CHAR_BIT;					\
    check(stdc_leading_zeros_##suffix(v), ref_leading(v, 0), "stdc_leading_zeros_" #suffix, v); \
    check(stdc_leading_ones_##suffix(v), ref_leading(v, 1), "stdc_leading_ones_" #suffix, v); \
    check(stdc_trailing_zeros_##suffix(v), ref_trailing(v, 0), "stdc_trailing_zeros_" #suffix, v); \
    check(stdc_trailing_ones_##suffix(v), ref_trailing(v, 1), "stdc_trailing_ones_" #suffix, v); \
    check(stdc_first_leading_zero_##suffix(v), ref_first_leading(v, 0), "stdc_first_leading_zero_" #suffix, v); \
    check(stdc_first_leading_one_##suffix(v), ref_first_leading(v, 1), "stdc_first_leading_one_" #suffix, v); \
    check(stdc_first_trailing_zero_##suffix(v), ref_first_trailing(v, 0), "stdc_first_trailing_zero_" #suffix, v); \
    check(stdc_first_trailing_one_##suffix(v), ref_first_trailing(v, 1), "stdc_first_trailing_one_" #suffix, v); \
    check(stdc_count_zeros_##suffix(v), ref_count(v, 0), "stdc_count_zeros_" #suffix, v); \
    check(stdc_count_ones_##suffix(v), ref_count(v, 1), "stdc_count_ones_" #suffix, v); \
    check(stdc_has_single_bit_##suffix(v), ref_count(v, 1) == 1, "stdc_has_single_bit_" #suffix, v); \
    check(stdc_bit_width_##suffix(v), ref_bit_width(v), "stdc_bit_width_" #suffix, v); \
    check(stdc_bit_floor_##suffix(v), ref_bit_floor(v), "stdc_bit_floor_" #suffix, v); \
    if (ref_bit_ceil(v) != 0)						\
      check(stdc_bit_ceil_##suffix(v), ref_bit_ceil(v), "stdc_bit_ceil_" #suffix, v); \
    check(stdc_leading_zeros(v), stdc_leading_zeros_##suffix(v), "stdc_leading_zeros", v); \
    check(stdc_count_ones(v), stdc_count_ones_##suffix(v), "stdc_count_ones", v); \
    check(stdc_bit_width(v), stdc_bit_width_##suffix(v), "stdc_bit_width", v); \
    check(stdc_bit_floor(v), stdc_bit_floor_##suffix(v), "stdc_bit_floor", v); \
  } while (0)

static void wide(u64 x)
{
  CHECK_TYPE(unsigned int, ui, x);
  CHECK_TYPE(unsigned long, ul, x);
  CHECK_TYPE(unsigned long long, ull, x);
}

int main(void)
{
  for (unsigned x = 0; x <= UCHAR_MAX; x++)
    CHECK_TYPE(unsigned char, uc, x);
  for (unsigned long x = 0; x <= USHRT_MAX; x++)
    CHECK_TYPE(unsigned short, us, x);

  wide(0);
  wide(~0ULL);
  for (unsigned i = 0; i < 64; i++)
    {
      u64 b = 1ULL << i;
      wide(b);
      wide(b - 1);
      wide(b + 1);
      wide(~b);
    }

  if (stdc_bit_width(0x80u) != 8 || !stdc_has_single_bit((unsigned char)0x80))
    {
      printf("FAIL: type-generic forms on constants\n");
      failures++;
    }

  return failures;
}
