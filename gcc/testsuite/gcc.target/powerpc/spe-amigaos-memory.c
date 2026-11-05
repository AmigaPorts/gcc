/* SPE intrinsic addresses must describe byte offsets to the optimizer.  */
/* { dg-do compile { target powerpc*-*-amigaos* } } */
/* { dg-options "-mcpu=8548 -O2" } */
/* { dg-final { scan-assembler "evstdd" } } */
/* { dg-final { scan-assembler "evldd" } } */

#include <spe.h>

unsigned long long
store_readback (void)
{
  unsigned long long buffer[8] = { 0 };
  __ev_stdd (__ev_create_u32 (1, 2), buffer, 3);
  return buffer[3];
}

unsigned long long
load_readback (void)
{
  unsigned long long buffer[8] = { 0 };
  buffer[3] = 0x123456789abcdef0ULL;
  return __ev_convert_u64 (__ev_ldd (buffer, 3));
}
