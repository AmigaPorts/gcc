/* m68k: n * 4 in DImode is add.l/addx.l twice.  addx.l leaves Z set when
   its result is zero, so the flags after it do not describe the high word.
   The test of the high word against zero must not be dropped (bbb mask
   tracking and final's REG_BIT_MASK compare elimination).  */

#include <stddef.h>

__attribute__((noinline)) int
ov (size_t n)
{
  ptrdiff_t c;
  return __builtin_mul_overflow (n, 4, &c);
}

int
main (void)
{
  if (ov (0) || ov (1) || ov (31) || ov (0x1fffffff))
    __builtin_abort ();
  if (__SIZEOF_SIZE_T__ == 4 && (!ov (0x20000000) || !ov ((size_t) -1)))
    __builtin_abort ();
  return 0;
}
