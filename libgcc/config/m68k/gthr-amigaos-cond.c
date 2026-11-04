/* The condition variables of gthr-amigaos.h on m68k.
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

#include "gthr-amigaos.h"
#include "gthr-amigaos-int.h"

extern struct ExecBase *SysBase;

/* Condition variables: a FIFO of waiters on their own stacks, as the
   benaphore's.  The queue is only touched under Forbid(), and a waiter is
   signalled once, after it is taken off it.  */

void
__gthread_cond_enqueue (__gthread_cond_t *cond, struct __gthread_cond_waiter *w)
{
  w->next = 0;
  w->task = SysBase->ThisTask;
  w->signalled = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
  if (cond->__tail)
    cond->__tail->next = w;
  else
    cond->__head = w;
  cond->__tail = w;
#pragma GCC diagnostic pop
}

void
__gthread_cond_dequeue (__gthread_cond_t *cond, struct __gthread_cond_waiter *w)
{
  struct __gthread_cond_waiter **link, *prev = 0;

  for (link = &cond->__head; *link; prev = *link, link = &(*link)->next)
    if (*link == w)
      {
	*link = w->next;
	if (cond->__tail == w)
	  cond->__tail = prev;
	return;
      }
}

static void
wake (__gthread_cond_t *cond)
{
  struct __gthread_cond_waiter *w = cond->__head;

  cond->__head = w->next;
  if (!cond->__head)
    cond->__tail = 0;
  w->signalled = 1;
  Signal (w->task, SIGF_SINGLE);
}

int
__gthread_cond_signal (__gthread_cond_t *cond)
{
  Forbid ();
  if (cond->__head)
    wake (cond);
  Permit ();
  return 0;
}

int
__gthread_cond_broadcast (__gthread_cond_t *cond)
{
  Forbid ();
  while (cond->__head)
    wake (cond);
  Permit ();
  return 0;
}

/* The unlock can't miss a signal: it happens under the Forbid() in which
   we queue ourselves, and Wait() breaks it only once we sleep.  */
int
__gthread_cond_wait (__gthread_cond_t *cond, __gthread_mutex_t *mx)
{
  struct __gthread_cond_waiter w;

  Forbid ();
  __gthread_cond_enqueue (cond, &w);
  benaphore_unlock (mx);
  while (!w.signalled)
    Wait (SIGF_SINGLE);
  Permit ();
  benaphore_lock (mx);
  return 0;
}

int
__gthread_cond_wait_recursive (__gthread_cond_t *cond,
			       __gthread_recursive_mutex_t *mx)
{
  unsigned long count = mx->__count;
  struct __gthread_cond_waiter w;

  Forbid ();
  __gthread_cond_enqueue (cond, &w);
  mx->__owner = 0;
  mx->__count = 0;
  benaphore_unlock (&mx->__b);
  while (!w.signalled)
    Wait (SIGF_SINGLE);
  Permit ();
  benaphore_lock (&mx->__b);
  mx->__owner = SysBase->ThisTask;
  mx->__count = count;
  return 0;
}

