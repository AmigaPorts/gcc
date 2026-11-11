/* { dg-do compile } */
/* { dg-options "-Os -m68000 -fomit-frame-pointer" } */
/* An interrupt handler saves only the registers it uses.  In a loop that
   never returns, d0 holds x and d1 is free but not saved, so the test of
   q stays cmpa.w #0,a0 rather than move.l a0,d1.  */

extern int *volatile p;
extern volatile int cnt;

__attribute__((interrupt)) void
handler (void)
{
  int x = cnt;
  for (;;)
    {
      int *q = p;
      if (q)
	*q = x;
      else
	cnt = x;
    }
}

/* { dg-final { scan-assembler-not {move\.l a[0-7],d1} } } */
/* { dg-final { scan-assembler-not {d1,-\(sp\)} } } */
