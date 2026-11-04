/* The keys of gthr-amigaos.h on m68k.
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
#include <proto/exec.h>

#include "gthr-amigaos.h"
#include "gthr-amigaos-int.h"

extern struct ExecBase *SysBase;

/* Keys.  Each task that sets one gets a block with a value per key, found
   in a hash table by its Task address.  A block is never unlinked or freed
   while the program runs: when its thread ends it is only marked free, for
   a later task in the same bucket.  So looking one up needs no lock, only
   atomic links and owners, and a task only ever writes the values in its
   own block.  */

#define KEYS_MAX 32
#define BUCKETS 16

struct key_block {
  struct key_block *_Atomic next;
  struct Task *_Atomic task;
  const void *values[KEYS_MAX];
};

static struct key_block *_Atomic buckets[BUCKETS];
static void (*key_destroy[KEYS_MAX]) (void *);
static bool key_used[KEYS_MAX];
static benaphore_t key_lock;

static struct key_block *_Atomic *
bucket (struct Task *task)
{
  return &buckets[((unsigned long) task >> 4) % BUCKETS];
}

static struct key_block *
find_block (struct Task *task)
{
  struct key_block *b;

  for (b = *bucket (task); b; b = b->next)
    if (b->task == task)
      return b;
  return 0;
}

int
__gthread_key_create (__gthread_key_t *key, void (*destroy) (void *))
{
  unsigned int i, j;
  struct key_block *b;

  benaphore_lock (&key_lock);
  for (i = 0; i < KEYS_MAX && key_used[i]; i++)
    ;
  if (i == KEYS_MAX)
    {
      benaphore_unlock (&key_lock);
      return EAGAIN;
    }
  /* A deleted key's values stay behind: clear them for the new one.  No
     task has this key yet, so none writes the slot meanwhile.  */
  for (j = 0; j < BUCKETS; j++)
    for (b = buckets[j]; b; b = b->next)
      b->values[i] = 0;
  key_destroy[i] = destroy;
  key_used[i] = true;
  benaphore_unlock (&key_lock);
  *key = i;
  return 0;
}

int
__gthread_key_delete (__gthread_key_t key)
{
  if (key >= KEYS_MAX)
    return EINVAL;
  benaphore_lock (&key_lock);
  key_used[key] = false;
  key_destroy[key] = 0;
  benaphore_unlock (&key_lock);
  return 0;
}

void *
__gthread_getspecific (__gthread_key_t key)
{
  struct key_block *b = find_block (SysBase->ThisTask);

  return b && key < KEYS_MAX ? (void *) b->values[key] : 0;
}

int
__gthread_setspecific (__gthread_key_t key, const void *value)
{
  struct Task *me = SysBase->ThisTask;
  struct key_block *_Atomic *head = bucket (me);
  struct key_block *b;

  if (key >= KEYS_MAX)
    return EINVAL;
  if (!(b = find_block (me)))
    {
      Forbid ();
      for (b = *head; b; b = b->next)
	if (!b->task)
	  {
	    b->task = me;
	    break;
	  }
      Permit ();
      if (!b)
	{
	  if (!(b = AllocMem (sizeof (*b), MEMF_ANY | MEMF_CLEAR)))
	    return ENOMEM;
	  b->task = me;
	  Forbid ();
	  b->next = *head;
	  *head = b;
	  Permit ();
	}
    }
  b->values[key] = value;
  return 0;
}

/* At the end of a thread: run its keys' destructors as pthreads do, at
   most four rounds while they set new values, then free its block.  The
   newest key goes first, since its values may live in an older one's:
   libstdc++'s thread_local destructors run on objects in emutls's.  */
void
__gthread_amigaos_end_keys (struct Task *me)
{
  struct key_block *b = find_block (me);
  int round, i;
  bool again;

  if (!b)
    return;
  for (round = 0; round < 4; round++)
    {
      again = false;
      for (i = KEYS_MAX - 1; i >= 0; i--)
	{
	  void (*destroy) (void *) = key_destroy[i];
	  void *value = (void *) b->values[i];

	  if (value && destroy && key_used[i])
	    {
	      b->values[i] = 0;
	      destroy (value);
	      again = true;
	    }
	}
      if (!again)
	break;
    }
  for (i = 0; i < KEYS_MAX; i++)
    b->values[i] = 0;
  b->task = 0;
}

static void __attribute__ ((destructor))
free_keys (void)
{
  struct key_block *b, *next;
  int i;

  for (i = 0; i < BUCKETS; i++)
    {
      for (b = buckets[i]; b; b = next)
	{
	  next = b->next;
	  FreeMem (b, sizeof (*b));
	}
      buckets[i] = 0;
    }
}

