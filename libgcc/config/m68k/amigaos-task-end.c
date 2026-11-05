/* The start and end of a task that another thread library started, for
   AmigaOS.
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

/* The amigaos thread model keeps each task's keys, and with them its
   thread-local variables, in a block that the task gets as it starts.  A
   thread library that starts tasks of its own, such as libpthread, calls
   these for each:
   - __gthread_amigaos_task_new() in the creator, before it starts the
     task, and fails the start if it fails: it keeps a block ready;
   - __gthread_amigaos_task_start() first thing in the new task, before
     any code that may use a thread-local variable;
   - __gthread_amigaos_task_cancel() in the creator instead, if the task
     didn't start after all, so that the block it kept goes back;
   - __gthread_amigaos_task_end() as the task ends, after its own
     destructors, so that those of its keys run and its block goes back.

   They are in libgcc with every thread model, so that the caller links
   with any: the key code sets the hooks as the program starts, and
   without it, or with another model, there is nothing to do.  */

struct Task;

int (*__gthread_amigaos_task_new_hook) (void);
void (*__gthread_amigaos_task_start_hook) (void);
void (*__gthread_amigaos_task_cancel_hook) (void);
void (*__gthread_amigaos_task_end_hook) (struct Task *);

int __stdargs
__gthread_amigaos_task_new (void)
{
  if (__gthread_amigaos_task_new_hook)
    return __gthread_amigaos_task_new_hook ();
  return 0;
}

void __stdargs
__gthread_amigaos_task_start (void)
{
  if (__gthread_amigaos_task_start_hook)
    __gthread_amigaos_task_start_hook ();
}

void __stdargs
__gthread_amigaos_task_cancel (void)
{
  if (__gthread_amigaos_task_cancel_hook)
    __gthread_amigaos_task_cancel_hook ();
}

void __stdargs
__gthread_amigaos_task_end (struct Task *task)
{
  if (__gthread_amigaos_task_end_hook)
    __gthread_amigaos_task_end_hook (task);
}
