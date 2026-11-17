/* { dg-do run } */
/* Identical code folding must not merge a normal function into an
   entrypoint one with the same body: the entrypoint saves no call-saved
   register.  */

extern void abort (void);

/* call_kept (fn, d2, d3, d4, a2) loads the four registers, calls FN and
   returns a mask of the registers that came back changed: 1 for d2, 2 for
   d3, 4 for d4 and 8 for a2.  */
extern long call_kept (void (*) (void), long, long, long, long);

__asm__ ("	.text\n"
	 "	.even\n"
	 "	.globl	_call_kept\n"
	 "_call_kept:\n"
	 "	movem.l	d2-d4/a2,-(sp)\n"
	 "	move.l	20(sp),a0\n"
	 "	move.l	24(sp),d2\n"
	 "	move.l	28(sp),d3\n"
	 "	move.l	32(sp),d4\n"
	 "	move.l	36(sp),a2\n"
	 "	jsr	(a0)\n"
	 "	moveq	#0,d0\n"
	 "	cmp.l	24(sp),d2\n"
	 "	jeq	1f\n"
	 "	addq.l	#1,d0\n"
	 "1:	cmp.l	28(sp),d3\n"
	 "	jeq	2f\n"
	 "	addq.l	#2,d0\n"
	 "2:	cmp.l	32(sp),d4\n"
	 "	jeq	3f\n"
	 "	addq.l	#4,d0\n"
	 "3:	cmp.l	36(sp),a2\n"
	 "	jeq	4f\n"
	 "	addq.l	#8,d0\n"
	 "4:	movem.l	(sp)+,d2-d4/a2\n"
	 "	rts\n");

int g[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
long sum;

__attribute__ ((noipa)) void
use (int x)
{
  sum += x;
}

/* The same body twice.  Eight values live across the calls to use () take
   the call-saved registers.  */
#define BODY							\
  {								\
    int a = g[0], b = g[1], c = g[2], d = g[3];		\
    int e = g[4], f = g[5], h = g[6], i = g[7];		\
    use (a);							\
    use (a + b + c + d + e + f + h + i);			\
    g[0] = b * c;						\
    g[1] = d * e * f * h * i;					\
  }

/* Never called: only its body is the same as normal's.  */
__entrypoint void ep (void) BODY

/* Must give d2-d7/a2-a6 back unchanged.  */
void normal (void) BODY

int
main (void)
{
  if (call_kept (normal, 0x12345678L, 0x23456789L, 0x3456789aL,
		 0x456789abL) != 0)
    abort ();
  return 0;
}
