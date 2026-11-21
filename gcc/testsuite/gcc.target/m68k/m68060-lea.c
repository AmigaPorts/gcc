/* A lea computes an address without accessing memory: on the 68060 one with
   a displacement pairs like an ALU instruction, and one with an index
   register goes in the primary pipeline alone.  */
/* { dg-do compile } */
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

/* lea (400,a0),a0 and lea (a0,a0.l*2),a0.  */
/* { dg-final { scan-rtl-dump {a\d=a\d\+0x190 +:m68060_pOEP\|m68060_sOEP\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {a\d=a\d<<0x1\+a\d +:m68060_single\n} "sched2" } } */
