/* A lea computes an address without accessing memory, and pairs like an ALU
   instruction on the 68060, unless the secondary pipeline doesn't take its
   address (MC68060 User's Manual, 10.1.3): (d16,An) and the brief index
   format (d8,An,Xn) pair, and the full format, here a 32-bit displacement
   with a suppressed base register, goes in the primary pipeline.  */
/* { dg-do compile } */
/* { dg-skip-if "a4-relative or pc-relative addresses" { *-*-* } { "-fbaserel*" "-resident*" "-mpcrel" } { "" } } */
/* { dg-options "-O2 -m68060 -fomit-frame-pointer -fsched-verbose=1 -fdump-rtl-sched2" } */

struct s
{
  int a[100];
  int b[100];
};

void
disp (struct s *p, int n)
{
  int *q = p->b;

  do
    *q++ = n;
  while (--n);
}

void
indexed (int *d, const int *s, int n)
{
  const int *e = s + n;

  do
    *d++ = *s++ * 3;
  while (s != e);
}

static int table[16];

int *
full (int i)
{
  return &table[i & 7];
}

/* lea (400,a0),a0, lea (a0,a0.l*2),a0 and lea _table(d0.l*4),a0.  */
/* { dg-final { scan-rtl-dump {a\d=a\d\+0x190 +:m68060_pOEP\|m68060_sOEP\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {a\d=a\d<<0x1\+a\d +:m68060_pOEP\|m68060_sOEP\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {a\d=d\d<<0x2\+`table' +:m68060_pOEP\n} "sched2" } } */
