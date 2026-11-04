/* { dg-do run } */
/* <benaphore.h> with one task: trylock, lock and unlock, the recursive
   variant, all-zeros initialization, and the slow paths called directly.
   Contention needs several tasks, which the simulator doesn't run.  */
#include <stdlib.h>
#include <string.h>
#include <exec/tasks.h>
#include <proto/exec.h>
#include <benaphore.h>

/* As libgcc lays out a sleeping task's entry in the queue.  */
struct __benaphore_waiter {
  struct __benaphore_waiter *next;
  struct Task *task;
  volatile int granted;
};

static benaphore_t zero_b;			/* in BSS */
static benaphore_recursive_t zero_r;
static benaphore_t init_b = BENAPHORE_INITIALIZER;
static benaphore_recursive_t init_r = BENAPHORE_RECURSIVE_INITIALIZER;

int
main (void)
{
  benaphore_t b;
  struct __benaphore_waiter w;
  int i;

  if (!benaphore_trylock (&zero_b) || benaphore_trylock (&zero_b))
    abort ();
  benaphore_unlock (&zero_b);
  if (!benaphore_trylock (&zero_b))
    abort ();
  benaphore_unlock (&zero_b);

  if (!benaphore_trylock (&init_b))
    abort ();
  benaphore_unlock (&init_b);

  memset (&b, 0, sizeof (b));
  for (i = 0; i < 1000; i++)
    {
      benaphore_lock (&b);
      benaphore_unlock (&b);
    }
  if (b.__locked || b.__head || b.__tail)
    abort ();
  benaphore_lock (&b);
  if (benaphore_trylock (&b))
    abort ();
  benaphore_unlock (&b);

  /* recursive: the owner counts */
  benaphore_recursive_lock (&zero_r);
  benaphore_recursive_lock (&zero_r);
  if (!benaphore_recursive_trylock (&zero_r) || zero_r.__count != 3)
    abort ();
  benaphore_recursive_unlock (&zero_r);
  benaphore_recursive_unlock (&zero_r);
  if (!zero_r.__b.__locked || zero_r.__count != 1)
    abort ();
  benaphore_recursive_unlock (&zero_r);
  if (zero_r.__b.__locked || zero_r.__owner || zero_r.__count)
    abort ();

  if (!benaphore_recursive_trylock (&init_r))
    abort ();
  benaphore_recursive_unlock (&init_r);
  if (init_r.__b.__locked)
    abort ();

  /* The slow paths, which take the lock as <benaphore.h> passes it: on a
     free lock, lock takes it; with this task queued, wake hands it over.  */
  memset (&b, 0, sizeof (b));
  __benaphore_lock_slow (&b);
  if (!b.__locked || b.__head)
    abort ();
  benaphore_unlock (&b);

  w.next = 0;
  w.task = FindTask (0);
  w.granted = 0;
  b.__head = b.__tail = &w;
  __benaphore_wake (&b);
  if (!w.granted || !b.__locked || b.__head || b.__tail)
    abort ();
  SetSignal (0, SIGF_SINGLE);
  benaphore_unlock (&b);
  if (b.__locked)
    abort ();

  return 0;
}
