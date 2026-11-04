/* What the gthr-amigaos*.c files on m68k share.
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

/* A task waiting on a condition variable, on its own stack.  */
struct __gthread_cond_waiter {
  struct __gthread_cond_waiter *next;
  struct Task *task;
  bool signalled;
};

/* Under Forbid().  */
void __gthread_cond_enqueue (__gthread_cond_t *, struct __gthread_cond_waiter *);
void __gthread_cond_dequeue (__gthread_cond_t *, struct __gthread_cond_waiter *);

/* At the end of a thread: run its key destructors and free its block.  */
struct Task;
void __gthread_amigaos_end_keys (struct Task *);
