/* { dg-do compile { target powerpc*-*-amigaos* } } */
/* { dg-options "-mcpu=8548 -O2" } */
/* { dg-final { scan-assembler "evaddw" } } */
/* { dg-final { scan-assembler "evlhhesplat" } } */

#include <spe.h>

__ev64_opaque__ add (__ev64_opaque__ a, __ev64_opaque__ b)
{
  return __ev_addw (a, b);
}

__ev64_opaque__ load_halfword (unsigned short *p)
{
  return __ev_lhhesplat (p, 7);
}
