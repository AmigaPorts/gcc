/* Instructions the 68060's secondary pipeline doesn't take (MC68060 User's
   Manual, Table 10-2): MOVE #<imm>,<mem> is pOEP-until-last, so it goes in
   the primary pipeline with a secondary instruction beside it, and SWAP is
   pOEP-only, alone.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68060 -fomit-frame-pointer -fsched-verbose=1 -fdump-rtl-sched2" } */

void
store_imm (int *p, int *q, int a, int b)
{
  *p = 0x12345678;
  *q = a + b;
}

unsigned
swap16 (unsigned x, unsigned y)
{
  return ((x << 16) | (x >> 16)) + (y << 1);
}

/* { dg-final { scan-rtl-dump {\[a\d\]=0x12345678 +:\(m68060_pOEP\+m68060_mem\)\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {d\d=d\d<-<0x10 +:m68060_single\n} "sched2" } } */
