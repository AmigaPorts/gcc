/* The 68060 issues two independent simple instructions in one cycle, one in
   each pipeline, so sched2 puts two such register-to-register instructions
   in the same cycle.  A pattern that emits two instructions, asr.l and
   roxr.l for a long long shifted right by one, takes two cycles.  */
/* { dg-do compile } */
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

long long
shift (long long x)
{
  return x >> 1;
}

/* { dg-final { scan-rtl-dump {;;\s+(\d+)--> [^\n]*:m68060_pOEP\|m68060_sOEP\n;;\s+\1--> [^\n]*:m68060_pOEP\|m68060_sOEP\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {>>0x1 +:m68060_single\*2} "sched2" } } */
