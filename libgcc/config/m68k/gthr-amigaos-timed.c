/* The timed waits of gthr-amigaos.h on m68k, apart so that only programs
   that use them link timer.device and gettimeofday().
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

#include <errno.h>
#include <sys/time.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/tasks.h>
#include <devices/timer.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "gthr-amigaos.h"
#include "gthr-amigaos-int.h"

extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;

/* Microseconds from now until the deadline, which is on the same clock as
   gettimeofday(), as libstdc++'s system_clock is; 0 if it has passed.  */
static unsigned long long
until (const __gthread_time_t *deadline)
{
  struct timeval now;
  long long us;

  gettimeofday (&now, 0);
  us = (deadline->__sec - now.tv_sec) * 1000000LL
       + deadline->__nsec / 1000 - now.tv_usec;
  return us > 0 ? us : 0;
}

int
__gthread_cond_timedwait (__gthread_cond_t *cond, __gthread_mutex_t *mx,
			  const __gthread_time_t *abs_timeout)
{
  unsigned long long us = until (abs_timeout);
  struct __gthread_cond_waiter w;
  struct timerequest *io;
  struct MsgPort *port;
  ULONG timer_sig;
  int ret = 0;

  if (!us)
    return ETIMEDOUT;
  if (!(port = CreateMsgPort ()))
    return ENOMEM;
  if (!(io = (struct timerequest *) CreateIORequest (port, sizeof (*io))))
    {
      DeleteMsgPort (port);
      return ENOMEM;
    }
  if (OpenDevice ((CONST_STRPTR) TIMERNAME, UNIT_MICROHZ, &io->tr_node, 0))
    {
      DeleteIORequest (&io->tr_node);
      DeleteMsgPort (port);
      return ENOMEM;
    }
  io->tr_node.io_Command = TR_ADDREQUEST;
  io->tr_time.tv_secs = us / 1000000;
  io->tr_time.tv_micro = us % 1000000;
  timer_sig = 1UL << port->mp_SigBit;

  /* As __gthread_cond_wait(), and a wakeup can only come while we are in
     Wait(), which returns it with the timer's if both came.  */
  Forbid ();
  SendIO (&io->tr_node);
  __gthread_cond_enqueue (cond, &w);
  benaphore_unlock (mx);
  while (!w.signalled)
    if (Wait (SIGF_SINGLE | timer_sig) & timer_sig)
      break;
  if (!w.signalled)
    {
      __gthread_cond_dequeue (cond, &w);
      ret = ETIMEDOUT;
    }
  Permit ();

  if (!CheckIO (&io->tr_node))
    AbortIO (&io->tr_node);
  WaitIO (&io->tr_node);
  CloseDevice (&io->tr_node);
  DeleteIORequest (&io->tr_node);
  DeleteMsgPort (port);
  benaphore_lock (mx);
  return ret;
}

/* The benaphore can't wait with a timeout, so try once a tick (1/50 s)
   until the deadline.  */
int
__gthread_mutex_timedlock (__gthread_mutex_t *mx,
			   const __gthread_time_t *abs_timeout)
{
  while (!benaphore_trylock (mx))
    {
      if (!until (abs_timeout))
	return ETIMEDOUT;
      Delay (1);
    }
  return 0;
}

int
__gthread_recursive_mutex_timedlock (__gthread_recursive_mutex_t *mx,
				     const __gthread_time_t *abs_timeout)
{
  while (!benaphore_recursive_trylock (mx))
    {
      if (!until (abs_timeout))
	return ETIMEDOUT;
      Delay (1);
    }
  return 0;
}
