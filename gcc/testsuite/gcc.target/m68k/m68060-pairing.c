/* The 68060 issues two independent simple instructions in one cycle, one in
   each pipeline, so sched2 puts a register-to-register instruction and
   another one that can go in the secondary pipeline in the same cycle, and a
   clr.w to memory, which has no immediate, goes in either pipeline.  A
   pattern that emits two instructions, asr.l and roxr.l or add.l and addx.l
   for a long long shifted by one, takes two cycles.  */
/* { dg-do compile } */
/* { dg-skip-if "a4-relative or pc-relative addresses, register arguments, 16-bit int" { *-*-* } { "-fbaserel*" "-resident*" "-mpcrel" "-mregparm*" "-msasregparm" "-mshort" } { "" } } */
/* { dg-options "-O2 -m68060 -fsched-verbose=1 -fdump-rtl-sched2" } */

int
pair (int a, int b, int c, int d)
{
  a += 1;
  b ^= c;
  c -= d;
  d |= a;
  return a + b + c + d;
}

void
clear (short *p, int *q, int a, int b)
{
  *p = 0;
  *q = a + b;
}

long long
shift (long long x)
{
  return x >> 1;
}

long long
shift_left (long long x)
{
  return x << 1;
}

/* { dg-final { scan-rtl-dump {;;\s+(\d+)--> [^\n]*:m68060_pOEP\|m68060_sOEP\n;;\s+\1--> [^\n]*m68060_sOEP} "sched2" } } */
/* { dg-final { scan-rtl-dump {\[%?a\d\]=0 +:\(m68060_pOEP\+m68060_mem\)\|\(m68060_sOEP\+m68060_mem\)\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {>>0x1 +:m68060_single\*2} "sched2" } } */
/* { dg-final { scan-rtl-dump {<<0x1 +:m68060_single\*2} "sched2" } } */
