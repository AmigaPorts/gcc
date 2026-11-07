/* { dg-do run } */
/* { dg-require-effective-target amigaos_gthreads } */
/* A program that starts no thread keeps its main task's tc_Launch, even
   with a key set: nothing costs at a task switch.  The hook goes on once
   the program announces a thread, and comes off at exit.  */
#include <stdlib.h>
#include <exec/tasks.h>
#include <proto/exec.h>

int __stdargs __gthread_key_create (unsigned int *, void (*) (void *));
int __stdargs __gthread_setspecific (unsigned int, const void *);
int __stdargs __gthread_amigaos_task_new (void);
void __stdargs __gthread_amigaos_task_cancel (void);
extern const void *const *__gthread_amigaos_values;

int
main (void)
{
  struct Task *me = FindTask (0);
  void (*launch) () = me->tc_Launch;
  UBYTE flags = me->tc_Flags & TF_LAUNCH;
  unsigned int key;
  static int value;

  if (__gthread_key_create (&key, 0)
      || __gthread_setspecific (key, &value))
    abort ();
  if (__gthread_amigaos_values[key] != &value)
    abort ();
  if (me->tc_Launch != launch || (me->tc_Flags & TF_LAUNCH) != flags)
    abort ();

  if (__gthread_amigaos_task_new ())
    abort ();
  __gthread_amigaos_task_cancel ();
  if (me->tc_Launch == launch || !(me->tc_Flags & TF_LAUNCH))
    abort ();
  if (__gthread_amigaos_values[key] != &value)
    abort ();
  return 0;
}
