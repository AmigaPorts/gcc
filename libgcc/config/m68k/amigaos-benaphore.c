/* The out-of-line half of <benaphore.h> for AmigaOS.
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

/* The queue of sleeping tasks, which is only touched under Forbid().  */

#include <exec/execbase.h>
#include <exec/tasks.h>
#include <proto/exec.h>
#include <benaphore.h>

/* On the stack of the task that sleeps.  */
struct __benaphore_waiter {
	struct __benaphore_waiter *next;
	struct Task *task;
	volatile int granted;
};

void __benaphore_lock_slow(benaphore_t *b)
{
	struct __benaphore_waiter w;

	w.next = 0;
	w.task = SysBase->ThisTask;
	w.granted = 0;

	Forbid();
	if (!benaphore_trylock(b)) {
		/* w is on our stack, but out of the queue before we return: the
		   task that grants us the lock dequeues us first.  */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
		if (b->__tail)
			b->__tail->next = &w;
		else
			b->__head = &w;
		b->__tail = &w;
#pragma GCC diagnostic pop
		/* SIGF_SINGLE is what exec's semaphores wait on; a task can't be
		   in both at once.  Wait() breaks the Forbid() while it sleeps and
		   restores it, and a stray signal only goes round the loop.  */
		while (!w.granted)
			Wait(SIGF_SINGLE);
	}
	Permit();
}

/* After an unlock found a sleeper: take the lock on behalf of the first one
   and hand it over.  If another task took the lock in between, it wakes the
   sleeper when it unlocks.  */
void __benaphore_wake(benaphore_t *b)
{
	struct __benaphore_waiter *w;

	Forbid();
	w = b->__head;
	if (w && benaphore_trylock(b)) {
		b->__head = w->next;
		if (!b->__head)
			b->__tail = 0;
		w->granted = 1;
		Signal(w->task, SIGF_SINGLE);
	}
	Permit();
}
