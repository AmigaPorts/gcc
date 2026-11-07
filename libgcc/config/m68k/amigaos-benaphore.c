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

/* The queue of sleeping tasks, see amigaos-benaphore-int.h.  Both slow
   paths run with the lock held, so they write the longword back with bit 0
   set.  */

#include <exec/execbase.h>
#include <exec/tasks.h>
#include <proto/exec.h>
#include "amigaos-benaphore-int.h"

static inline void set_last(benaphore_t *b, struct __benaphore_waiter *w)
{
	b->__last = (struct __benaphore_waiter *)
		((unsigned long) w | __BENAPHORE_LOCKED);
}

void __benaphore_lock_slow(benaphore_t *b)
{
	struct __benaphore_waiter w, *l;

	w.task = SysBase->ThisTask;
	w.granted = false;

	Forbid();
	if (!benaphore_trylock(b)) {
		/* w is on our stack, but out of the queue before we return: the
		   task that grants us the lock dequeues us first.  */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
		l = __benaphore_last(b);
		if (l) {
			w.next = l->next;
			l->next = &w;
		} else
			w.next = &w;
		set_last(b, &w);
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
	struct __benaphore_waiter *w, *l;

	Forbid();
	l = __benaphore_last(b);
	if (l && benaphore_trylock(b)) {
		w = l->next;
		if (w == l)
			set_last(b, 0);
		else
			l->next = w->next;
		w->granted = true;
		Signal(w->task, SIGF_SINGLE);
	}
	Permit();
}
