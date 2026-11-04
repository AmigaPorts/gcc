/* The threads of gthr-amigaos.h on m68k.
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
#include <stddef.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "gthr-amigaos.h"
#include "gthr-amigaos-int.h"

extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;

/* Threads.  Each is a DOS process, and its __gthread_t is its Task.  */

struct thread {
  void *a4;			/* first: the entry code loads it */
  void *(*func) (void *);
  void *args;
  void *result;
  struct Task *task;
  struct Task *joiner;
  struct thread *next;
  volatile unsigned char done;
  unsigned char detached;
};

/* Only touched under Forbid().  */
static struct thread *threads;

_Static_assert (offsetof (struct ExecBase, ThisTask) == 276, "ThisTask");
_Static_assert (offsetof (struct Task, tc_UserData) == 88, "tc_UserData");
_Static_assert (offsetof (struct thread, a4) == 0, "a4");

void __gthread_amigaos_run (struct thread *) __attribute__ ((__stkparm__));

/* A new thread waits for __gthread_create() to put its struct thread in
   tc_UserData and signal it, then sets up the creator's a4, which
   -fbaserel code addresses its data with, before any C code runs.  */
__asm__ ("\t.text\n"
	 "\t.even\n"
	 "___gthread_amigaos_entry:\n"
	 "\tmove.l 4.w,a6\n"
	 "\tmoveq #16,d0\n"		/* SIGF_SINGLE */
	 "\tjsr -318(a6)\n"		/* Wait() */
	 "\tmove.l 276(a6),a0\n"	/* ThisTask */
	 "\tmove.l 88(a0),a0\n"		/* tc_UserData */
	 "\tmove.l (a0),a4\n"
	 "\tmove.l a0,-(sp)\n"
	 "\tjsr ___gthread_amigaos_run\n"
	 "\taddq.l #4,sp\n"
	 "\trts\n");
/* A function, so that -fbaserel doesn't address it relative to a4.  */
extern void __gthread_amigaos_entry (void);

static void
unlink_thread (struct thread *t)
{
  struct thread **link;

  for (link = &threads; *link; link = &(*link)->next)
    if (*link == t)
      {
	*link = t->next;
	return;
      }
}

static struct thread *
find_thread (struct Task *task)
{
  struct thread *t;

  for (t = threads; t; t = t->next)
    if (t->task == task)
      return t;
  return 0;
}

void
__gthread_amigaos_run (struct thread *t)
{
  t->task->tc_UserData = 0;
  t->result = t->func (t->args);
  __gthread_amigaos_end_keys (t->task);

  /* Stay in Forbid() until the process is gone: whoever we signal may
     free t and end the program.  */
  Forbid ();
  t->done = 1;
  if (t->detached)
    {
      unlink_thread (t);
      FreeVec (t);
    }
  else if (t->joiner)
    Signal (t->joiner, SIGF_SINGLE);
}

int
__gthread_create (__gthread_t *thread, void *(*func) (void *), void *args)
{
  struct Task *me = SysBase->ThisTask;
  unsigned long stack = (char *) me->tc_SPUpper - (char *) me->tc_SPLower;
  struct Process *proc;
  struct thread *t;
  BPTR in = 0, out = 0;

  if (!(t = AllocVec (sizeof (*t), MEMF_ANY | MEMF_CLEAR)))
    return EAGAIN;
  t->func = func;
  t->args = args;
  __asm__ ("move.l a4,%0" : "=g" (t->a4));

  /* As much stack as the creator, which C++ code needs more than the
     4000 bytes DOS gives by default.  */
  if (stack < 32768)
    stack = 32768;
  /* The creator's console, which the thread shares and must not close: by
     default a new process gets NIL: handles of its own, and a C library
     that sets up stdio on first use would bind it to them.  */
  if (me->tc_Node.ln_Type == NT_PROCESS)
    {
      in = Input ();
      out = Output ();
    }
  proc = CreateNewProcTags (NP_Entry, (ULONG) __gthread_amigaos_entry,
			    NP_StackSize, stack,
			    NP_Name, (ULONG) "gthread",
			    in ? NP_Input : TAG_IGNORE, in,
			    in ? NP_CloseInput : TAG_IGNORE, FALSE,
			    out ? NP_Output : TAG_IGNORE, out,
			    out ? NP_CloseOutput : TAG_IGNORE, FALSE,
			    TAG_END);
  if (!proc)
    {
      FreeVec (t);
      return EAGAIN;
    }

  Forbid ();
  t->task = &proc->pr_Task;
  t->next = threads;
  threads = t;
  Permit ();
  proc->pr_Task.tc_UserData = t;
  Signal (&proc->pr_Task, SIGF_SINGLE);
  *thread = &proc->pr_Task;
  return 0;
}

int
__gthread_join (__gthread_t thread, void **value_ptr)
{
  struct Task *me = SysBase->ThisTask;
  struct thread *t;

  Forbid ();
  if (!(t = find_thread (thread)) || t->detached || t->joiner)
    {
      Permit ();
      return t ? EINVAL : ESRCH;
    }
  if (thread == me)
    {
      Permit ();
      return EDEADLK;
    }
  t->joiner = me;
  while (!t->done)
    Wait (SIGF_SINGLE);
  unlink_thread (t);
  Permit ();
  if (value_ptr)
    *value_ptr = t->result;
  FreeVec (t);
  return 0;
}

int
__gthread_detach (__gthread_t thread)
{
  struct thread *t;

  Forbid ();
  if (!(t = find_thread (thread)) || t->detached || t->joiner)
    {
      Permit ();
      return t ? EINVAL : ESRCH;
    }
  if (t->done)
    {
      unlink_thread (t);
      FreeVec (t);
    }
  else
    t->detached = 1;
  Permit ();
  return 0;
}

/* SetTaskPri() to the same priority puts us behind the other tasks of
   that priority and reschedules.  */
int
__gthread_yield (void)
{
  struct Task *me = SysBase->ThisTask;

  SetTaskPri (me, me->tc_Node.ln_Pri);
  return 0;
}
