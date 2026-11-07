/* Amiga register-parameter ABI: functions that only read their arguments
   in call-saved registers do not save them, and a forwarder that passes
   them on unchanged, long or short, tail-jumps.  Each call goes through
   call_kept, which loads d2, d3, d4 and a2 itself and compares all 32
   bits of each after the call, so a callee that changes one of them
   fails whatever the caller's register allocation.  */

/* { dg-do run } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */

extern void abort (void);

/* call_kept (fn, d2, d3, d4, a2, result) loads the four registers, calls
   FN, stores its result in *RESULT and returns a mask of the registers
   that came back changed: 1 for d2, 2 for d3, 4 for d4 and 8 for a2.  */
extern long call_kept (void *, long, long, long, long, long *);

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
	 "	move.l	40(sp),a0\n"
	 "	move.l	d0,(a0)\n"
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

__attribute__ ((noipa)) long
take3 (long a __asm ("d2"), long b __asm ("d3"), long c __asm ("a2"))
{
  return a * 10000 + b * 100 + c;
}

__attribute__ ((noipa)) long
forward3 (long a __asm ("d2"), long b __asm ("d3"), long c __asm ("a2"))
{
  return take3 (a, b, c);
}

__attribute__ ((noipa)) long
take_short (short w __asm ("d2"), unsigned char m __asm ("d4"))
{
  return w * 1000L + m;
}

__attribute__ ((noipa)) long
forward_short (short w __asm ("d2"), unsigned char m __asm ("d4"))
{
  return take_short (w, m);
}

__attribute__ ((noipa)) long
ident (long a __asm ("d2"))
{
  return a;
}

/* Writes part of d2 from d2: on a 68020, a bfins.  */
__attribute__ ((noipa)) long
insert_byte (long a __asm ("d2"))
{
  a = (a & 0x00ffffffL) | (a << 24);
  return ident (a);
}

/* Writes d3 through an asm clobber.  */
__attribute__ ((noipa)) long
clobber_d3 (long a __asm ("d3"))
{
  __asm__ volatile ("moveq #-1,%%d3" : : : "d3");
  return a + 1;
}

int
main (void)
{
  long r;

  if (call_kept (take3, 12, 34, 0x5a5a5a5aL, 56, &r) != 0
      || r != 123456)
    abort ();
  if (call_kept (forward3, 12, 34, 0x5a5a5a5aL, 56, &r) != 0
      || r != 123456)
    abort ();
  /* The upper bits of the narrow arguments must survive too.  */
  if (call_kept (forward_short, 0x5555fff4L, 0x11111111L, 0x123456c8L, 0,
		 &r) != 0
      || r != -11800)
    abort ();
  if (call_kept (insert_byte, 0x12345678L, 0, 0, 0, &r) != 0
      || r != 0x78345678L)
    abort ();
  if (call_kept (clobber_d3, 0, 41, 0, 0, &r) != 0
      || r != 42)
    abort ();
  return 0;
}
