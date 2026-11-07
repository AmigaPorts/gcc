/* The condition variables of <benaphore.h> for AmigaOS.
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

#include <exec/execbase.h>
#include <exec/tasks.h>
#include <proto/exec.h>

#include "amigaos-benaphore-int.h"

/* Condition variables: a FIFO of waiters on their own stacks, as the
   benaphore's.  The queue is only touched under Forbid(), and a waiter is
   signalled once, after it is taken off it.  */

void
__benaphore_cond_enqueue (benaphore_cond_t *c, struct __benaphore_cond_waiter *w)
{
  w->next = 0;
  w->task = SysBase->ThisTask;
  w->signalled = false;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
  if (c->__tail)
    c->__tail->next = w;
  else
    c->__head = w;
  c->__tail = w;
#pragma GCC diagnostic pop
}

void
__benaphore_cond_dequeue (benaphore_cond_t *c, struct __benaphore_cond_waiter *w)
{
  struct __benaphore_cond_waiter **link, *prev = 0;

  for (link = &c->__head; *link; prev = *link, link = &(*link)->next)
    if (*link == w)
      {
	*link = w->next;
	if (c->__tail == w)
	  c->__tail = prev;
	return;
      }
}

static void
wake (benaphore_cond_t *c)
{
  struct __benaphore_cond_waiter *w = c->__head;

  c->__head = w->next;
  if (!c->__head)
    c->__tail = 0;
  w->signalled = true;
  Signal (w->task, SIGF_SINGLE);
}

void
__benaphore_cond_signal (benaphore_cond_t *c)
{
  Forbid ();
  if (c->__head)
    wake (c);
  Permit ();
}

void
__benaphore_cond_broadcast (benaphore_cond_t *c)
{
  Forbid ();
  while (c->__head)
    wake (c);
  Permit ();
}

/* The unlock can't miss a signal: it happens under the Forbid() in which
   we queue ourselves, and Wait() breaks it only once we sleep.  */
void
__benaphore_cond_wait (benaphore_cond_t *c, benaphore_t *b)
{
  struct __benaphore_cond_waiter w;

  Forbid ();
  __benaphore_cond_enqueue (c, &w);
  benaphore_unlock (b);
  while (!w.signalled)
    Wait (SIGF_SINGLE);
  Permit ();
  benaphore_lock (b);
}
