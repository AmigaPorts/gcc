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

/* Under Forbid(), with the lock taken by someone else: join the queue and
   sleep until the lock is ours.  w is on our stack, but out of the queue
   before we return: the task that grants us the lock dequeues us first.  */
static void sleep_in_queue(benaphore_t *b, struct __benaphore_waiter *w)
{
	struct __benaphore_waiter *l;

	w->task = SysBase->ThisTask;
	w->granted = false;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
	l = __benaphore_last(b);
	if (l) {
		w->next = l->next;
		l->next = w;
	} else
		w->next = w;
	set_last(b, w);
#pragma GCC diagnostic pop
	/* SIGF_SINGLE is what exec's semaphores wait on; a task can't be in
	   both at once.  Wait() breaks the Forbid() while it sleeps and
	   restores it, and a stray signal only goes round the loop.  */
	while (!w->granted)
		Wait(SIGF_SINGLE);
}

/* Under Forbid(), with the lock taken on behalf of the first sleeper: take
   it off the queue and return it.  */
static struct __benaphore_waiter *take_first(benaphore_t *b,
					     struct __benaphore_waiter *l)
{
	struct __benaphore_waiter *w = l->next;

	if (w == l)
		set_last(b, 0);
	else
		l->next = w->next;
	return w;
}

void __benaphore_lock_slow(benaphore_t *b)
{
	struct __benaphore_waiter w;

	Forbid();
	if (!benaphore_trylock(b))
		sleep_in_queue(b, &w);
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
		w = take_first(b, l);
		w->granted = true;
		Signal(w->task, SIGF_SINGLE);
	}
	Permit();
}

/* Priority inheritance for the recursive benaphore, see <benaphore.h>.  The
   record of the raised task is only touched under Forbid().  */

/* Put back t's priority, unless t has changed it since it was raised.  */
static void put_back(struct Task *t, BYTE base, BYTE boost)
{
	if (t->tc_Node.ln_Pri == boost)
		SetTaskPri(t, base);
}

/* Under Forbid(): put back the task a sleeper raised, if any.  */
static void restore(benaphore_recursive_t *r)
{
	struct Task *t = r->__boosted;

	if (t) {
		r->__boosted = 0;
		put_back(t, r->__base_pri, r->__boost_pri);
	}
}

/* Under Forbid(): if it was the running task that a sleeper raised, forget
   the record and return 1 with the priorities for put_back(), which the
   task calls after the Permit(): only then is a task it hands the lock to
   ready, and dropping below that one switches to it at once.  Another
   raised task is put back at once, as it might end before a later call.  */
static int take_own(benaphore_recursive_t *r, BYTE *base, BYTE *boost)
{
	if (r->__boosted != SysBase->ThisTask) {
		restore(r);
		return 0;
	}
	r->__boosted = 0;
	*base = r->__base_pri;
	*boost = r->__boost_pri;
	return 1;
}

/* Raise the owner o, if it is below pri: up to the highest of its sleepers,
   never down.  A raised task that isn't the owner any more is put back
   first.  The owner is 0 for the two instructions after a task has taken
   the lock and before it says so: a sleeper that comes then can't raise it.  */
static void raise_owner(benaphore_recursive_t *r, struct Task *o, BYTE pri)
{
	if (r->__boosted && r->__boosted != o)
		restore(r);
	if (o && o->tc_Node.ln_Pri < pri) {
		if (!r->__boosted) {
			r->__boosted = o;
			r->__base_pri = o->tc_Node.ln_Pri;
		}
		r->__boost_pri = pri;
		SetTaskPri(o, pri);
	}
}

void __benaphore_recursive_lock_slow(benaphore_recursive_t *r)
{
	struct __benaphore_waiter w;

	Forbid();
	if (!benaphore_trylock(&r->__b)) {
		raise_owner(r, __atomic_load_n(&r->__owner, __ATOMIC_RELAXED),
			    SysBase->ThisTask->tc_Node.ln_Pri);
		sleep_in_queue(&r->__b, &w);
	}
	Permit();
}

/* As __benaphore_wake(), and the task a sleeper raised gets its priority
   back.  The sleepers still queued raise the new owner: it was first in the
   queue, not the highest, and it owns the lock from now, before it runs.  */
void __benaphore_recursive_wake(benaphore_recursive_t *r)
{
	struct __benaphore_waiter *w, *l, *p;
	BYTE pri, base, boost;
	int own;

	Forbid();
	own = take_own(r, &base, &boost);
	l = __benaphore_last(&r->__b);
	if (l && benaphore_trylock(&r->__b)) {
		w = take_first(&r->__b, l);
		__atomic_store_n(&r->__owner, w->task, __ATOMIC_RELAXED);
		if ((l = __benaphore_last(&r->__b))) {
			pri = -128;
			p = l;
			do {
				p = p->next;
				if (p->task->tc_Node.ln_Pri > pri)
					pri = p->task->tc_Node.ln_Pri;
			} while (p != l);
			raise_owner(r, w->task, pri);
		}
		w->granted = true;
		Signal(w->task, SIGF_SINGLE);
	}
	Permit();
	if (own)
		put_back(SysBase->ThisTask, base, boost);
}

/* A recursive owner about to release the lock in a condition wait.  It is
   the owner no more, so no sleeper can raise it again.  */
void __benaphore_recursive_unboost(benaphore_recursive_t *r)
{
	BYTE base, boost;
	int own;

	Forbid();
	own = take_own(r, &base, &boost);
	Permit();
	if (own)
		put_back(SysBase->ThisTask, base, boost);
}
