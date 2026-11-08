/* A fast mutex for AmigaOS tasks.
   Copyright (C) 2026 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/* A benaphore: a mutex for AmigaOS tasks on one CPU that takes two inline
   instructions to lock and three to unlock when nobody else wants it, and
   sleeps when someone does.

   The lock is bit 0 of the benaphore's one longword, taken with bset and
   released with bclr.  On a single CPU one instruction can't be interrupted
   by a task switch or an interrupt, so bset is an atomic test-and-set
   (Linux/m68k's test_and_set_bit() is the same bset), and it needs no locked
   bus cycle, so unlike tas and cas it works on the 68000 and in chip RAM.

   A task that finds the lock taken queues itself and sleeps in Wait(); the
   task that releases it hands it to the first in the queue, so sleepers are
   served in order, though a task that tries the lock while a release is
   under way can get it first.  The rest of the longword points to the last
   of them, whose next is the first: a circular queue, which takes new
   sleepers at the end and hands the lock to the front, each in O(1).  The
   sleepers are on their tasks' stacks, which are even, so bit 0 of their
   address is free for the lock.  The queue is only touched under Forbid(),
   out of line.
   The name is from BeOS, which paired an atomic fast path with a semaphore
   in the same way:
   https://www.haiku-os.org/legacy-docs/benewsletter/Issue1-26.html

   All zeros is a valid unlocked benaphore.  Like any lock that can sleep,
   it is for tasks, not interrupt code, and it needs a single CPU.  It has
   no priority inheritance, as exec's semaphores don't.

   What a 68000 takes when nobody else wants the lock, in cycles from the
   instruction timings, with its address in a register:

	benaphore_lock()		 30
	benaphore_unlock()		 40
	benaphore_recursive_lock()	125, or 100 nested
	benaphore_recursive_unlock()	110, or 50 nested

   The recursive one reads ThisTask, compares the owner and counts, so it
   costs about three times as much: take it only where the same task can
   take the lock again.  When another task holds the lock, Wait() and a
   task switch come on top.  */

#ifndef _BENAPHORE_H
#define _BENAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

struct __benaphore_waiter;

typedef struct {
	/* the last sleeper, or 0; bit 0 is the lock.  Read atomically
	   outside the lock and Forbid(). */
	struct __benaphore_waiter *__last;
} benaphore_t;

#define BENAPHORE_INITIALIZER { 0 }

/* The byte of __last that holds bit 0: m68k is big-endian.  */
#define __BENAPHORE_LOCK_BYTE(__b) \
	(((unsigned char *) &(__b)->__last)[3])

/* Taken again by the task that holds it, it counts instead of blocking.
   Other tasks read __owner without the lock, so all accesses to it are
   atomic. */
typedef struct {
	benaphore_t __b;
	void *__owner;			/* struct Task * */
	unsigned long __count;
} benaphore_recursive_t;

#define BENAPHORE_RECURSIVE_INITIALIZER { BENAPHORE_INITIALIZER, 0, 0 }

/* The slow paths take the lock in a0 whatever -mregparm the caller was
   built with.  The asm names are the ones -mmangle-regparm gives them, so
   every caller links to the one libgcc build, with or without it.  Gas
   takes the @ only in quotes.  They never throw, which C++ callers that
   can't throw must know to stay free of exception handling.  */
#define __BENAPHORE_ABI __attribute__((__regparm__(1), __nothrow__))
void __benaphore_lock_slow(benaphore_t *__b) __asm__("\"@__benaphore_lock_slow\"") __BENAPHORE_ABI;
void __benaphore_wake(benaphore_t *__b) __asm__("\"@__benaphore_wake\"") __BENAPHORE_ABI;

/* In C++ the inline functions have external linkage, as gthr-posix.h's,
   so that inline functions of a module may use them. */
#ifdef __cplusplus
#define __BENAPHORE_INLINE inline __attribute__((__always_inline__))
#else
#define __BENAPHORE_INLINE static __inline__ __attribute__((__always_inline__))
#endif

/* 1 if the lock was free and is now ours, 0 if someone holds it. */
__BENAPHORE_INLINE int benaphore_trylock(benaphore_t *__b)
{
	unsigned char __was;

	__asm__ __volatile__ ("bset #0,%1\n\tsne %0"
			      : "=d" (__was), "+m" (__BENAPHORE_LOCK_BYTE(__b))
			      : : "cc", "memory");
	return !__was;
}

/* bset leaves Z set if the bit was clear: branch on that, rather than
   turning it into a value with sne and testing that again.  Over the slow
   call, not out to it: a short branch, which gcc can't move away and
   lengthen, and the same code at -O2 and -Os.  The byte is an input and
   the "memory" clobber covers the write: -mno-lra can't take an asm goto
   with outputs. */
__BENAPHORE_INLINE void benaphore_lock(benaphore_t *__b)
{
	__asm__ __volatile__ goto ("bset #0,%0\n\tjeq %l[__free]"
				   : : "m" (__BENAPHORE_LOCK_BYTE(__b))
				   : "cc", "memory" : __free);
	__benaphore_lock_slow(__b);
__free:;
}

/* Release first, then look for sleepers: a task that queues itself does so
   only after seeing the bit set under Forbid(), so either it saw the bit
   clear and took the lock, or it is in the queue by the time we look.  The
   longword is nonzero with no sleepers only if another task took the lock
   in between, which costs a call that finds nobody to wake.  We no longer
   hold the lock, so the look is an atomic load. */
__BENAPHORE_INLINE void benaphore_unlock(benaphore_t *__b)
{
	__asm__ __volatile__ ("bclr #0,%0" : "+m" (__BENAPHORE_LOCK_BYTE(__b))
			      : : "cc", "memory");
	if (__atomic_load_n(&__b->__last, __ATOMIC_RELAXED))
		__benaphore_wake(__b);
}

/* exec's ThisTask, at the offset struct ExecBase has always had it, so that
   this header doesn't need the NDK's.  SysBase goes by another name here:
   C++ code declares it with C++ linkage through <proto/exec.h>, or with C
   linkage inside extern "C", and either would clash with ours.  A plain
   load: while a task runs, ThisTask is that task. */
extern void *__benaphore_sysbase __asm__("_SysBase");
#define __BENAPHORE_THIS_TASK() \
	(*(void **) ((char *) __benaphore_sysbase + 276))

__BENAPHORE_INLINE void benaphore_recursive_lock(benaphore_recursive_t *__r)
{
	void *__me = __BENAPHORE_THIS_TASK();

	/* Only this task can have made itself the owner. */
	if (__atomic_load_n(&__r->__owner, __ATOMIC_RELAXED) == __me) {
		__r->__count++;
		return;
	}
	benaphore_lock(&__r->__b);
	__atomic_store_n(&__r->__owner, __me, __ATOMIC_RELAXED);
	__r->__count = 1;
}

__BENAPHORE_INLINE int benaphore_recursive_trylock(benaphore_recursive_t *__r)
{
	void *__me = __BENAPHORE_THIS_TASK();

	if (__atomic_load_n(&__r->__owner, __ATOMIC_RELAXED) == __me) {
		__r->__count++;
		return 1;
	}
	if (!benaphore_trylock(&__r->__b))
		return 0;
	__atomic_store_n(&__r->__owner, __me, __ATOMIC_RELAXED);
	__r->__count = 1;
	return 1;
}

__BENAPHORE_INLINE void benaphore_recursive_unlock(benaphore_recursive_t *__r)
{
	if (--__r->__count == 0) {
		__atomic_store_n(&__r->__owner, (void *) 0, __ATOMIC_RELAXED);
		benaphore_unlock(&__r->__b);
	}
}

/* A deadline, on the clock of gettimeofday(): a struct timespec, but the
   same whatever time_t the C library has.  __nsec is in [0, 1000000000),
   as libstdc++ fills it and as pthreads check it. */
typedef struct {
	long long __sec;
	long __nsec;
} benaphore_time_t;

/* A condition variable: a FIFO of the tasks that wait on it, each on its
   own stack.  All zeros is a valid one. */
struct __benaphore_cond_waiter;
typedef struct {
	struct __benaphore_cond_waiter *__head, *__tail;
} benaphore_cond_t;

#define BENAPHORE_COND_INITIALIZER { 0, 0 }

/* libgcc builds these once, so they take their arguments on the stack
   whatever -mregparm and -mmangle-regparm the caller uses.  The timed ones
   return 0, or ETIMEDOUT once the deadline has passed.  They are apart from
   the others, so that only programs that use them link DOSBase and
   gettimeofday().

   FIXME: the deadline is coarse, whatever nanoseconds it has.  A timed wait
   sleeps on a clone of dos.library's timer request, timer.device's
   UNIT_VBLANK, which counts vertical blanks: it can end late by up to one,
   20 ms at 50 Hz (PAL) or 17 ms at 60 Hz (NTSC).  It is compared against
   gettimeofday(), which in newlib and libnix moves in DOS ticks of 1/50 s,
   so a finer timer would gain nothing until the clock is finer too: see
   AmigaPorts/m68k-amigaos-gcc#324.  A release or a signal wakes the waiter
   at once, without waiting for the timer. */
#define __BENAPHORE_STACK_ABI __stdargs __attribute__((__nothrow__))
int __benaphore_lock_timed(benaphore_t *__b, const benaphore_time_t *__deadline) __BENAPHORE_STACK_ABI;
void __benaphore_cond_signal(benaphore_cond_t *__c) __BENAPHORE_STACK_ABI;
void __benaphore_cond_broadcast(benaphore_cond_t *__c) __BENAPHORE_STACK_ABI;
void __benaphore_cond_wait(benaphore_cond_t *__c, benaphore_t *__b) __BENAPHORE_STACK_ABI;
int __benaphore_cond_timedwait(benaphore_cond_t *__c, benaphore_t *__b,
			       const benaphore_time_t *__deadline) __BENAPHORE_STACK_ABI;

/* As benaphore_lock(), but gives up at the deadline.  A task that holds the
   lock already waits for the deadline: use the recursive one if it can. */
__BENAPHORE_INLINE int benaphore_timedlock(benaphore_t *__b,
					   const benaphore_time_t *__deadline)
{
	if (benaphore_trylock(__b))
		return 0;
	return __benaphore_lock_timed(__b, __deadline);
}

__BENAPHORE_INLINE int benaphore_recursive_timedlock(benaphore_recursive_t *__r,
						     const benaphore_time_t *__deadline)
{
	void *__me = __BENAPHORE_THIS_TASK();
	int __err;

	if (__atomic_load_n(&__r->__owner, __ATOMIC_RELAXED) == __me) {
		__r->__count++;
		return 0;
	}
	if ((__err = benaphore_timedlock(&__r->__b, __deadline)))
		return __err;
	__atomic_store_n(&__r->__owner, __me, __ATOMIC_RELAXED);
	__r->__count = 1;
	return 0;
}

/* Nothing to do when nobody waits, the usual case.  A waiter joins the
   queue in the same Forbid() as it releases the lock, so a task that
   signals with the lock held can't miss it; one that signals without the
   lock and finds the queue empty signalled before the wait began. */
__BENAPHORE_INLINE void benaphore_cond_signal(benaphore_cond_t *__c)
{
	if (__atomic_load_n(&__c->__head, __ATOMIC_RELAXED))
		__benaphore_cond_signal(__c);
}

__BENAPHORE_INLINE void benaphore_cond_broadcast(benaphore_cond_t *__c)
{
	if (__atomic_load_n(&__c->__head, __ATOMIC_RELAXED))
		__benaphore_cond_broadcast(__c);
}

/* Release the lock, which the caller holds, and sleep until signalled; then
   take the lock again.  The release and the start of the wait are one step
   for the tasks that signal. */
__BENAPHORE_INLINE void benaphore_cond_wait(benaphore_cond_t *__c, benaphore_t *__b)
{
	__benaphore_cond_wait(__c, __b);
}

/* As benaphore_cond_wait(), until the deadline at most.  The lock is taken
   again either way. */
__BENAPHORE_INLINE int benaphore_cond_timedwait(benaphore_cond_t *__c, benaphore_t *__b,
						const benaphore_time_t *__deadline)
{
	return __benaphore_cond_timedwait(__c, __b, __deadline);
}

/* The recursive lock is released whatever its count, and taken back with
   it.  Until then the owner is nobody, so that another task can take it;
   only the holder writes the count. */
__BENAPHORE_INLINE void benaphore_cond_wait_recursive(benaphore_cond_t *__c,
						      benaphore_recursive_t *__r)
{
	unsigned long __count = __r->__count;

	__atomic_store_n(&__r->__owner, (void *) 0, __ATOMIC_RELAXED);
	__benaphore_cond_wait(__c, &__r->__b);
	__atomic_store_n(&__r->__owner, __BENAPHORE_THIS_TASK(), __ATOMIC_RELAXED);
	__r->__count = __count;
}

__BENAPHORE_INLINE int benaphore_cond_timedwait_recursive(benaphore_cond_t *__c,
							  benaphore_recursive_t *__r,
							  const benaphore_time_t *__deadline)
{
	unsigned long __count = __r->__count;
	int __err;

	__atomic_store_n(&__r->__owner, (void *) 0, __ATOMIC_RELAXED);
	__err = __benaphore_cond_timedwait(__c, &__r->__b, __deadline);
	__atomic_store_n(&__r->__owner, __BENAPHORE_THIS_TASK(), __ATOMIC_RELAXED);
	__r->__count = __count;
	return __err;
}

#ifdef __cplusplus
}
#endif

#endif /* _BENAPHORE_H */
