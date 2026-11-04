/* The end of a task that another thread library started, for AmigaOS.
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
   thread-local variables, by the task's address.  A thread library that
   starts tasks of its own, such as libpthread, calls this as each one
   ends, so that its destructors run and a later task at the same address
   starts afresh.

   It is in libgcc with every thread model, so that the caller links with
   any: the key code sets the hook when the first key is created, and
   until then, or with another model, there is nothing to forget.  */

struct Task;

void (*__gthread_amigaos_task_end_hook) (struct Task *);

void __stdargs
__gthread_amigaos_task_end (struct Task *task)
{
  if (__gthread_amigaos_task_end_hook)
    __gthread_amigaos_task_end_hook (task);
}
