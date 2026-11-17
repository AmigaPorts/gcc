/* { dg-do run } */
/* A saveallregs function restores every register before it returns, so
   it can't tail-call: the restore would overwrite the callee's register
   arguments, and the address of an indirect jump.  */

extern void abort (void);

/* The asm below uses them, which -flto can't see.  */
__attribute__ ((used, externally_visible)) long count, seen;

/* Both keep all registers.  touch () counts its calls; check () stores
   its argument from d1.  */
__saveallregs void touch (void);
__saveallregs void check (long x __asm ("d1"));

__asm__ ("	.text\n"
	 "	.even\n"
	 "	.globl	_touch\n"
	 "_touch:\n"
	 "	addq.l	#1,_count\n"
	 "	rts\n"
	 "	.globl	_check\n"
	 "_check:\n"
	 "	move.l	d1,_seen\n"
	 "	rts\n");

/* With -fbaserel on the 68000, a tail call would jump through a0.  */
__attribute__ ((noipa)) __saveallregs void
call_touch (void)
{
  touch ();
}

/* A tail call would lose the 42 in d1.  */
__attribute__ ((noipa)) __saveallregs void
call_check (void)
{
  check (42);
}

int
main (void)
{
  call_touch ();
  if (count != 1)
    abort ();
  call_check ();
  if (seen != 42)
    abort ();
  return 0;
}
