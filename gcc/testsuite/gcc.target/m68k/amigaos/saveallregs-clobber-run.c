/* { dg-do run } */
/* A saveallregs function gives back every register, also when it calls a
   function that may change d0/d1/a0/a1: a tail call would leave those as
   the callee left them.  */

extern void abort (void);

/* call_kept (fn, d1, a1) loads d1 and a1, calls FN and returns a mask of
   the registers that came back changed: 1 for d1, 2 for a1.  */
extern long call_kept (void (*) (void), long, long);

/* clobber () changes d1 and a1, which a normal function may do.  */
extern void clobber (void);

__asm__ ("	.text\n"
	 "	.even\n"
	 "	.globl	_call_kept\n"
	 "_call_kept:\n"
	 "	move.l	a2,-(sp)\n"
	 "	move.l	8(sp),a2\n"
	 "	move.l	12(sp),d1\n"
	 "	move.l	16(sp),a1\n"
	 "	jsr	(a2)\n"
	 "	moveq	#0,d0\n"
	 "	cmp.l	12(sp),d1\n"
	 "	jeq	1f\n"
	 "	addq.l	#1,d0\n"
	 "1:	cmp.l	16(sp),a1\n"
	 "	jeq	2f\n"
	 "	addq.l	#2,d0\n"
	 "2:	move.l	(sp)+,a2\n"
	 "	rts\n"
	 "	.globl	_clobber\n"
	 "_clobber:\n"
	 "	moveq	#-1,d1\n"
	 "	move.l	d1,a1\n"
	 "	rts\n");

__saveallregs void
wrapper (void)
{
  clobber ();
}

int
main (void)
{
  if (call_kept (wrapper, 0x12345678L, 0x23456789L) != 0)
    abort ();
  return 0;
}
