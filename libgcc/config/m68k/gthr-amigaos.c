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

/* Keys.  Each task of the program gets a block with a value per key, and
   while it runs, __gthread_amigaos_values points to the block's values:
   the inline __gthread_getspecific() just reads them through it.  The
   block holds the task's tc_Launch, which exec calls every time the task
   gets the CPU, and which sets the pointer; a task that already had one
   still gets it called first.  exec calls it in supervisor mode with a4
   not the program's, so it is written here with the absolute addresses,
   which are also right for a resident program's copy of the data.

   As only tc_Launch sets the pointer, a task without one would read the
   values of the task that ran before it.  So each task of the program
   gets its block before another one can run:
   - the main task when it first sets a key, or before it starts its first
     thread;
   - a thread before it runs any code of its creator's:
     __gthread_amigaos_new_keys() in the creator keeps a block ready for
     it, and __gthread_amigaos_start_keys(), the first thing the thread
     does, puts it on.
   __gthread_create() calls them, and libpthread calls them through
   amigaos-task-end.c.  Until the main task has a block, the pointer
   points to zeros: it is the only task, and has set no key.

   When a thread ends, __gthread_amigaos_end_keys() runs its destructors,
   takes its tc_Launch off, and puts its block on the free list for a
   later thread.  Blocks are only freed at exit.  A task that fails to
   start leaves its block on the list.  */

#define KEYS_MAX 32

/* The words of launch[] after the longs, so that these stay aligned.  The
   task is read without key_lock, to tell whether the block is ours.  */
struct key_block {
  struct key_block *next;		/* every block */
  struct key_block *next_free;
  struct Task *_Atomic task;		/* or null on the free list */
  const void *values[KEYS_MAX];
  void (*old_launch) ();
  /* jsr old.L, then move.l #values,__gthread_amigaos_values.L and rts */
  unsigned short launch[9];
  unsigned char old_flags;
};

static const void *no_values[KEYS_MAX];
const void *const *__gthread_amigaos_values = no_values;

#define BLOCK(v) \
  ((struct key_block *) ((char *) (v) - offsetof (struct key_block, values)))

/* The blocks, the free ones and how many of those are kept for threads
   that are starting, under key_lock.  */
static struct key_block *blocks, *free_blocks;
static unsigned int free_count, starting;

static void (*key_destroy[KEYS_MAX]) (void *);
static bool key_used[KEYS_MAX];
static benaphore_t key_lock;

static unsigned short *
put32 (unsigned short *p, const void *v)
{
  p[0] = (unsigned long) v >> 16;
  p[1] = (unsigned long) v;
  return p + 2;
}

/* Supervisor() code that makes the CPU see the hook just written: a 68020
   or 68030 clears its caches through CACR, a 68040 or 68060 pushes them.
   Opcodes, so that they assemble for the 68000 too.  */
__asm__ ("\t.text\n"
	 "\t.even\n"
	 "_gthr_flush_030:\n"
	 "\t.short 0x4e7a,0x0002\n"	/* movec cacr,d0 */
	 "\tor.w #0x0808,d0\n"		/* CD and CI */
	 "\t.short 0x4e7b,0x0002\n"	/* movec d0,cacr */
	 "\trte\n"
	 "_gthr_flush_040:\n"
	 "\t.short 0xf4f8\n"		/* cpusha bc */
	 "\trte\n");
extern void gthr_flush_030 (void) __asm__ ("_gthr_flush_030");
extern void gthr_flush_040 (void) __asm__ ("_gthr_flush_040");

/* CacheClearU() is from exec V37; before it, the same by hand, so that
   this works from Kickstart 1.3 on.  */
static void
flush_caches (void)
{
  UWORD cpu = SysBase->AttnFlags;

  if (SysBase->LibNode.lib_Version >= 37)
    CacheClearU ();
  else if (cpu & AFF_68040)
    Supervisor ((ULONG (*) ()) gthr_flush_040);
  else if (cpu & AFF_68020)
    Supervisor ((ULONG (*) ()) gthr_flush_030);
}

/* This task's block, or null.  */
static struct key_block *
my_block (void)
{
  const void *const *v = __gthread_amigaos_values;

  if (v != no_values && BLOCK (v)->task == SysBase->ThisTask)
    return BLOCK (v);
  return 0;
}

/* Make sure that a block is free beyond those kept for starting threads.
   Under key_lock.  */
static int
reserve (void)
{
  struct key_block *b;

  if (free_count > starting)
    return 0;
  if (!(b = AllocMem (sizeof (*b), MEMF_ANY | MEMF_CLEAR)))
    return ENOMEM;
  b->next = blocks;
  blocks = b;
  b->next_free = free_blocks;
  free_blocks = b;
  free_count++;
  return 0;
}

/* Give the running task a free block and its tc_Launch.  Under key_lock,
   with a block free.  */
static void
hook (void)
{
  struct Task *me = SysBase->ThisTask;
  struct key_block *b = free_blocks;
  unsigned short *p = b->launch;

  free_blocks = b->next_free;
  free_count--;
  b->task = me;
  Forbid ();
  b->old_flags = me->tc_Flags & TF_LAUNCH;
  b->old_launch = b->old_flags ? me->tc_Launch : 0;
  if (b->old_launch)
    {
      *p++ = 0x4eb9;			/* jsr old.L */
      p = put32 (p, (const void *) b->old_launch);
    }
  *p++ = 0x23fc;			/* move.l #values,...L */
  p = put32 (put32 (p, b->values), &__gthread_amigaos_values);
  *p = 0x4e75;				/* rts */
  flush_caches ();
  me->tc_Launch = (void (*) ()) b->launch;
  me->tc_Flags |= TF_LAUNCH;
  __gthread_amigaos_values = b->values;
  Permit ();
}

/* Give the running task its old tc_Launch back.  */
static void
unhook (struct key_block *b)
{
  struct Task *me = SysBase->ThisTask;

  Forbid ();
  me->tc_Launch = b->old_launch;
  me->tc_Flags = (me->tc_Flags & ~TF_LAUNCH) | b->old_flags;
  Permit ();
}

/* Before the running task starts a task: give it a block, if it has none
   yet, and keep one ready for the new task.  */
int
__gthread_amigaos_new_keys (void)
{
  int err = 0;

  benaphore_lock (&key_lock);
  if (!my_block () && !(err = reserve ()))
    hook ();
  if (!err && !(err = reserve ()))
    starting++;
  benaphore_unlock (&key_lock);
  return err;
}

/* The first thing in a new task: put on the block kept for it.  Only a task
   that no __gthread_amigaos_new_keys() announced can find none, and then
   only when out of memory.  */
void
__gthread_amigaos_start_keys (void)
{
  benaphore_lock (&key_lock);
  if (starting)
    starting--;
  if (!reserve ())
    hook ();
  benaphore_unlock (&key_lock);
}

/* In the creator, when the task that __gthread_amigaos_new_keys() kept a
   block for didn't start: the block is free for the next one.  */
void
__gthread_amigaos_cancel_keys (void)
{
  benaphore_lock (&key_lock);
  if (starting)
    starting--;
  benaphore_unlock (&key_lock);
}

int
__gthread_key_create (__gthread_key_t *key, void (*destroy) (void *))
{
  unsigned int i;
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
  for (b = blocks; b; b = b->next)
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

int
__gthread_setspecific (__gthread_key_t key, const void *value)
{
  struct key_block *b;
  int err = 0;

  if (key >= KEYS_MAX)
    return EINVAL;
  if (!(b = my_block ()))
    {
      benaphore_lock (&key_lock);
      if (!(err = reserve ()))
	hook ();
      benaphore_unlock (&key_lock);
      if (err)
	return err;
      b = my_block ();
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
  struct key_block *b = my_block ();
  int round, i;
  bool again;

  if (!b || me != SysBase->ThisTask)
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
  unhook (b);
  for (i = 0; i < KEYS_MAX; i++)
    b->values[i] = 0;
  benaphore_lock (&key_lock);
  b->task = 0;
  b->next_free = free_blocks;
  free_blocks = b;
  free_count++;
  benaphore_unlock (&key_lock);
}

static void __attribute__ ((constructor))
init_keys (void)
{
  __gthread_amigaos_task_new_hook = __gthread_amigaos_new_keys;
  __gthread_amigaos_task_start_hook = __gthread_amigaos_start_keys;
  __gthread_amigaos_task_cancel_hook = __gthread_amigaos_cancel_keys;
  __gthread_amigaos_task_end_hook = __gthread_amigaos_end_keys;
}

static void __attribute__ ((destructor))
free_keys (void)
{
  struct key_block *b, *next;

  /* The hooks are in the blocks.  Only this task's can still be called:
     every other thread has ended, as it must before the program does.  */
  if ((b = my_block ()))
    unhook (b);
  __gthread_amigaos_values = no_values;
  for (b = blocks; b; b = next)
    {
      next = b->next;
      FreeMem (b, sizeof (*b));
    }
  blocks = free_blocks = 0;
  free_count = starting = 0;
}
