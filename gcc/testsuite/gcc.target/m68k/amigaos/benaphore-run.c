/* { dg-do run } */
/* <benaphore.h> with one task: trylock, lock and unlock, the recursive
   variant, all-zeros initialization, and the slow paths called directly,
   with sleepers made up for the queue.  Contention needs several tasks,
   which the simulator doesn't run.  */
#include <stdlib.h>
#include <string.h>
#include <exec/tasks.h>
#include <proto/exec.h>
#include <benaphore.h>

/* As libgcc lays out a sleeping task's entry in the queue.  */
struct __benaphore_waiter {
  struct __benaphore_waiter *next;
  struct Task *task;
  bool granted;
};

/* Locked, with nobody in the queue.  */
#define HELD(b) ((unsigned long) (b).__last == 1)

static benaphore_t zero_b;			/* in BSS */
static benaphore_recursive_t zero_r;
static benaphore_t init_b = BENAPHORE_INITIALIZER;
static benaphore_recursive_t init_r = BENAPHORE_RECURSIVE_INITIALIZER;

int
main (void)
{
  benaphore_t b;
  struct __benaphore_waiter w, w2;
  int i;

  if (sizeof (benaphore_t) != 4)
    abort ();

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
  if (b.__last)
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
  if (!HELD (zero_r.__b) || zero_r.__count != 1)
    abort ();
  benaphore_recursive_unlock (&zero_r);
  if (zero_r.__b.__last || zero_r.__owner || zero_r.__count)
    abort ();

  if (!benaphore_recursive_trylock (&init_r))
    abort ();
  benaphore_recursive_unlock (&init_r);
  if (init_r.__b.__last)
    abort ();

  /* The slow paths, which take the lock as <benaphore.h> passes it: on a
     free lock, lock takes it; with this task queued, wake hands it over.  */
  memset (&b, 0, sizeof (b));
  __benaphore_lock_slow (&b);
  if (!HELD (b))
    abort ();
  benaphore_unlock (&b);

  w.next = &w;
  w.task = FindTask (0);
  w.granted = false;
  b.__last = &w;
  __benaphore_wake (&b);
  if (!w.granted || !HELD (b))
    abort ();
  SetSignal (0, SIGF_SINGLE);
  benaphore_unlock (&b);
  if (b.__last)
    abort ();

  /* Two sleepers, w first: the queue is circular, from the last one.  The
     unlock that wakes w finds w2 still queued and wakes it next.  */
  w.granted = w2.granted = false;
  w2.task = w.task;
  w.next = &w2;
  w2.next = &w;
  b.__last = &w2;
  __benaphore_wake (&b);
  if (!w.granted || w2.granted
      || (unsigned long) b.__last != ((unsigned long) &w2 | 1)
      || w2.next != &w2)
    abort ();
  benaphore_unlock (&b);
  if (!w2.granted || !HELD (b))
    abort ();
  SetSignal (0, SIGF_SINGLE);
  benaphore_unlock (&b);
  if (b.__last)
    abort ();

  return 0;
}
