/* What the amigaos-benaphore*.c files on m68k share.
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

#include <benaphore.h>

/* A task sleeping on a benaphore, on its own stack.  The benaphore's
   longword is the last sleeper's address with the lock in bit 0, and the
   last sleeper's next is the first.  Only touched under Forbid().  */
struct __benaphore_waiter {
	struct __benaphore_waiter *next;
	struct Task *task;
	bool granted;
};

#define __BENAPHORE_LOCKED 1UL

static inline struct __benaphore_waiter *__benaphore_last(benaphore_t *b)
{
	return (struct __benaphore_waiter *)
		((unsigned long) b->__last & ~__BENAPHORE_LOCKED);
}

/* A task waiting on a condition variable, on its own stack.  It is
   signalled once, after it is taken off the queue.  */
struct __benaphore_cond_waiter {
	struct __benaphore_cond_waiter *next;
	struct Task *task;
	bool signalled;
};

void __benaphore_cond_enqueue(benaphore_cond_t *c, struct __benaphore_cond_waiter *w);
void __benaphore_cond_dequeue(benaphore_cond_t *c, struct __benaphore_cond_waiter *w);
