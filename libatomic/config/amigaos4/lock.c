/* Copyright (C) 2012-2015 Free Software Foundation, Inc.
   Contributed by Richard Henderson <rth@redhat.com>.

   This file is part of the GNU Atomic Library (libatomic).

   Libatomic is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Libatomic is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include "libatomic_i.h"

#define UWORD UWORD_
#include <proto/exec.h>

/* The target page size.  Must be no larger than the runtime page size,
   lest locking fail with virtual address aliasing (i.e. a page mmaped
   at two locations).  */
#ifndef PAGE_SIZE
#define PAGE_SIZE	4096
#endif

/* The target cachline size.  This is an optimization; the padding that
   we're doing to avoid false sharing doesn't have to be this large, but
   it does have to be at least sizeof(pthread_mutex_t).  */
#ifndef CACHLINE_SIZE
#define CACHLINE_SIZE	64
#endif

/* The granularity at which we watch for shared access.  */
#ifndef WATCH_SIZE
#define WATCH_SIZE	CACHLINE_SIZE
#endif

struct lock
{
  struct SignalSemaphore sem;
  char initialized;
  char pad[sizeof(struct SignalSemaphore) + 1 < CACHLINE_SIZE
	   ? CACHLINE_SIZE - sizeof(struct SignalSemaphore) - 1
	   : 0];
};

/* TODO: Use a constructor to initialize the semaphore */
#define NLOCKS		(PAGE_SIZE / WATCH_SIZE)
static struct lock locks[NLOCKS];

static inline uintptr_t
addr_hash (void *ptr)
{
  return ((uintptr_t)ptr / WATCH_SIZE) % NLOCKS;
}

static void
libat_lock (struct lock *l)
{
  IExec->Forbid();
  if (!l->initialized)
  {
    IExec->InitSemaphore (&l->sem);
    l->initialized = 1;
  }
  IExec->Permit();
  IExec->ObtainSemaphore (&l->sem);
}

static void
libat_unlock (struct lock *l)
{
  IExec->ReleaseSemaphore (&l->sem);
}

void
libat_lock_1 (void *ptr)
{
  libat_lock (&locks[addr_hash (ptr)]);
}

void
libat_unlock_1 (void *ptr)
{
  libat_unlock (&locks[addr_hash (ptr)]);
}

void
libat_lock_n (void *ptr, size_t n)
{
  uintptr_t h = addr_hash (ptr);
  size_t i = 0;

  /* Don't lock more than all the locks we have.  */
  if (n > PAGE_SIZE)
    n = PAGE_SIZE;

  do
    {
      libat_lock (&locks[h]);
      if (++h == NLOCKS)
        h = 0;
      i += WATCH_SIZE;
    }
  while (i < n);
}

void
libat_unlock_n (void *ptr, size_t n)
{
  uintptr_t h = addr_hash (ptr);
  size_t i = 0;

  if (n > PAGE_SIZE)
    n = PAGE_SIZE;

  do
    {
      libat_unlock (&locks[h]);
      if (++h == NLOCKS)
        h = 0;
      i += WATCH_SIZE;
    }
  while (i < n);
}
