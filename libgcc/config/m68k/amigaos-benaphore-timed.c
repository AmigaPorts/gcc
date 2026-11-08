/* The timed waits of <benaphore.h> for AmigaOS, apart so that only
   programs that use them link DOSBase and gettimeofday().
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

/* A timed wait sleeps in the same queue as an untimed one, so the task
   that releases the lock or signals wakes it at once, and a timer request
   only ends the wait at the deadline.  */

#include <errno.h>
#include <sys/time.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/tasks.h>
#include <devices/timer.h>
#include <dos/dosextens.h>
#include <proto/exec.h>

#include "amigaos-benaphore-int.h"

extern struct DosLibrary *DOSBase;

/* A request for the deadline, on the waiting task's stack: each wait needs
   its own, since a task can have only one in flight per request.  A clone
   of dos.library's, which is already open, as libnix's sleep() and AROS's
   Delay() use it.  It replies on SIGF_SINGLE, which the task sleeps on
   anyway, so it needs no signal of its own and the wait loop sees either.  */
struct timer {
  struct MsgPort port;
  struct timerequest io;
};

/* 0 with the request ready to send, or ETIMEDOUT if the deadline has
   passed.  The deadline is on the clock of gettimeofday(), as libstdc++'s
   system_clock and pthreads' are.  Seconds and microseconds apart, so that
   no 64-bit division is needed.  */
static int
timer_init (struct timer *t, const benaphore_time_t *deadline)
{
  struct MsgPort *p = &t->port;
  struct timeval now;
  long long secs;
  long micro;

  gettimeofday (&now, 0);
  secs = deadline->__sec - now.tv_sec;
  micro = deadline->__nsec / 1000 - now.tv_usec;
  if (micro < 0)
    {
      micro += 1000000;
      secs--;
    }
  if (secs < 0 || (secs == 0 && micro == 0))
    return ETIMEDOUT;
  /* tr_time's seconds are 32 bits: wait no more than 68 years.  */
  if (secs > 0x7fffffff)
    secs = 0x7fffffff;

  p->mp_Node.ln_Type = NT_MSGPORT;
  p->mp_Flags = PA_SIGNAL;
  p->mp_SigBit = SIGB_SINGLE;
  p->mp_SigTask = SysBase->ThisTask;
  p->mp_MsgList.lh_Head = (struct Node *) &p->mp_MsgList.lh_Tail;
  p->mp_MsgList.lh_Tail = 0;
  p->mp_MsgList.lh_TailPred = (struct Node *) &p->mp_MsgList.lh_Head;

  t->io = *DOSBase->dl_TimeReq;
  t->io.tr_node.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
  t->io.tr_node.io_Message.mn_ReplyPort = p;
  t->io.tr_node.io_Command = TR_ADDREQUEST;
  t->io.tr_node.io_Flags = 0;
  t->io.tr_time.tv_secs = secs;
  t->io.tr_time.tv_micro = micro;
  return 0;
}

/* After the wait, whatever ended it.  Then no reply can come any more, and
   one that came after the last Wait() mustn't wake a later one early.  */
static void
timer_stop (struct timer *t)
{
  if (!CheckIO (&t->io.tr_node))
    AbortIO (&t->io.tr_node);
  WaitIO (&t->io.tr_node);
  SetSignal (0, SIGF_SINGLE);
}

/* Take w off the benaphore's circular queue, whose last sleeper the
   longword points to, keeping the lock bit as it is.  */
static void
dequeue (benaphore_t *b, struct __benaphore_waiter *w)
{
  unsigned long locked = (unsigned long) b->__last & __BENAPHORE_LOCKED;
  struct __benaphore_waiter *l = __benaphore_last (b), *p = l;

  while (p->next != w)
    p = p->next;
  if (p == w)
    l = 0;
  else
    {
      p->next = w->next;
      if (l == w)
	l = p;
    }
  b->__last = (struct __benaphore_waiter *) ((unsigned long) l | locked);
}

/* As __benaphore_lock_slow(), until the deadline.  */
int
__benaphore_lock_timed (benaphore_t *b, const benaphore_time_t *deadline)
{
  struct __benaphore_waiter w, *l;
  struct timer t;
  int err;

  if ((err = timer_init (&t, deadline)))
    return err;
  w.task = SysBase->ThisTask;
  w.granted = false;

  Forbid ();
  if (benaphore_trylock (b))
    {
      Permit ();
      return 0;
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
  l = __benaphore_last (b);
  if (l)
    {
      w.next = l->next;
      l->next = &w;
    }
  else
    w.next = &w;
  b->__last = (struct __benaphore_waiter *)
    ((unsigned long) &w | __BENAPHORE_LOCKED);
#pragma GCC diagnostic pop
  SendIO (&t.io.tr_node);
  while (!w.granted && !CheckIO (&t.io.tr_node))
    Wait (SIGF_SINGLE);
  if (!w.granted)
    {
      dequeue (b, &w);
      err = ETIMEDOUT;
    }
  Permit ();

  timer_stop (&t);
  return err;
}

/* As __benaphore_cond_wait(), until the deadline.  The lock is taken again
   either way, without a deadline, as POSIX says.  */
int
__benaphore_cond_timedwait (benaphore_cond_t *c, benaphore_t *b,
			    const benaphore_time_t *deadline)
{
  struct __benaphore_cond_waiter w;
  struct timer t;
  int err;

  if ((err = timer_init (&t, deadline)))
    return err;

  Forbid ();
  __benaphore_cond_enqueue (c, &w);
  benaphore_unlock (b);
  SendIO (&t.io.tr_node);
  while (!w.signalled && !CheckIO (&t.io.tr_node))
    Wait (SIGF_SINGLE);
  if (!w.signalled)
    {
      __benaphore_cond_dequeue (c, &w);
      err = ETIMEDOUT;
    }
  Permit ();

  timer_stop (&t);
  benaphore_lock (b);
  return err;
}
