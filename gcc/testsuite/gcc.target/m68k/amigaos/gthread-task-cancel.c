/* { dg-do run } */
/* { dg-require-effective-target amigaos_gthreads } */
/* A thread library that announces a task with __gthread_amigaos_task_new()
   and then fails to start it gives the block back with
   __gthread_amigaos_task_cancel(): the next announcement reuses it.
   Without the cancel, each one would keep another block for good.  */
#include <stdlib.h>
#include <exec/memory.h>
#include <proto/exec.h>

int __stdargs __gthread_amigaos_task_new (void);
void __stdargs __gthread_amigaos_task_cancel (void);

/* The key code, which sets the hooks the calls go through.  */
extern const void *const *__gthread_amigaos_values;

#define ROUNDS 1000

int
main (void)
{
  ULONG before;
  int i;

  if (!*(const void *const *volatile *) &__gthread_amigaos_values)
    abort ();

  /* The first gives this task a block of its own as well.  */
  if (__gthread_amigaos_task_new ())
    abort ();
  __gthread_amigaos_task_cancel ();

  before = AvailMem (MEMF_ANY);
  for (i = 0; i < ROUNDS; i++)
    {
      if (__gthread_amigaos_task_new ())
	abort ();
      __gthread_amigaos_task_cancel ();
    }
  /* A block per round would be well over 100 KB.  */
  if (before - AvailMem (MEMF_ANY) > 4096)
    abort ();
  return 0;
}
