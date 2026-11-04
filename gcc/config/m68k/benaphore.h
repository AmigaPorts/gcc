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

/* A benaphore: a mutex for AmigaOS tasks on one CPU that costs two inline
   instructions to take and two to release when nobody else wants it, and
   sleeps when someone does.

   The lock is bit 0 of the benaphore's one longword, taken with bset and
   released with bclr.  On a single CPU one instruction can't be interrupted
   by a task switch or an interrupt, so bset is an atomic test-and-set
   (Linux/m68k's test_and_set_bit() is the same bset), and it needs no locked
   bus cycle, so unlike tas and cas it works on the 68000 and in chip RAM.

   A task that finds the lock taken queues itself and sleeps in Wait(); the
   task that releases it hands it to the first in the queue, so waiters are
   served in order.  The rest of the longword points to the last of them,
   whose next is the first: a circular queue, which takes new sleepers at the
   end and hands the lock to the front, each in O(1).  The sleepers are on
   their tasks' stacks, which are even, so bit 0 of their address is free
   for the lock.  The queue is only touched under Forbid(), out of line.
   The name is from BeOS, which paired an atomic fast path with a semaphore
   in the same way:
   https://www.haiku-os.org/legacy-docs/benewsletter/Issue1-26.html

   All zeros is a valid unlocked benaphore.  Use it from tasks only, not from
   interrupt code, and only on a single CPU.  It has no priority
   inheritance, as exec's semaphores don't.  */

#ifndef _BENAPHORE_H
#define _BENAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

struct __benaphore_waiter;

typedef struct {
	/* the last sleeper, or 0; bit 0 is the lock */
	struct __benaphore_waiter *volatile __last;
} benaphore_t;

#define BENAPHORE_INITIALIZER { 0 }

/* The byte of __last that holds bit 0: m68k is big-endian.  */
#define __BENAPHORE_LOCK_BYTE(__b) \
	(((volatile unsigned char *) &(__b)->__last)[3])

/* Taken again by the task that holds it, it counts instead of blocking. */
typedef struct {
	benaphore_t __b;
	void *volatile __owner;		/* struct Task * */
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
#define __BENAPHORE_INLINE static __inline__
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

__BENAPHORE_INLINE void benaphore_lock(benaphore_t *__b)
{
	if (!benaphore_trylock(__b))
		__benaphore_lock_slow(__b);
}

/* Release first, then look for sleepers: a task that queues itself does so
   only after seeing the bit set under Forbid(), so either it saw the bit
   clear and took the lock, or it is in the queue by the time we look.  The
   longword is nonzero with no sleepers only if another task took the lock
   in between, which costs a call that finds nobody to wake. */
__BENAPHORE_INLINE void benaphore_unlock(benaphore_t *__b)
{
	__asm__ __volatile__ ("bclr #0,%0" : "+m" (__BENAPHORE_LOCK_BYTE(__b))
			      : : "cc", "memory");
	if (__b->__last)
		__benaphore_wake(__b);
}

/* exec's ThisTask, at the offset struct ExecBase has always had it, so that
   this header doesn't need the NDK's.  SysBase goes by another name here:
   C++ code declares it with C++ linkage through <proto/exec.h>, or with C
   linkage inside extern "C", and either would clash with ours. */
extern void *__benaphore_sysbase __asm__("_SysBase");
#define __BENAPHORE_THIS_TASK() \
	(*(void *volatile *) ((char *) __benaphore_sysbase + 276))

__BENAPHORE_INLINE void benaphore_recursive_lock(benaphore_recursive_t *__r)
{
	void *__me = __BENAPHORE_THIS_TASK();

	/* Only this task can have made itself the owner. */
	if (__r->__owner == __me) {
		__r->__count++;
		return;
	}
	benaphore_lock(&__r->__b);
	__r->__owner = __me;
	__r->__count = 1;
}

__BENAPHORE_INLINE int benaphore_recursive_trylock(benaphore_recursive_t *__r)
{
	void *__me = __BENAPHORE_THIS_TASK();

	if (__r->__owner == __me) {
		__r->__count++;
		return 1;
	}
	if (!benaphore_trylock(&__r->__b))
		return 0;
	__r->__owner = __me;
	__r->__count = 1;
	return 1;
}

__BENAPHORE_INLINE void benaphore_recursive_unlock(benaphore_recursive_t *__r)
{
	if (--__r->__count == 0) {
		__r->__owner = 0;
		benaphore_unlock(&__r->__b);
	}
}

#ifdef __cplusplus
}
#endif

#endif /* _BENAPHORE_H */
