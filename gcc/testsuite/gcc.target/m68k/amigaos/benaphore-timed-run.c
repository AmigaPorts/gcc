/* { dg-do run } */
/* The timed waits of <benaphore.h> and its condition variables with one
   task: every wait here ends at its deadline, or at a wakeup made up for
   the queue, so none is woken with its timer still pending.  */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <exec/tasks.h>
#include <proto/exec.h>
#include <benaphore.h>

/* As libgcc lays out a sleeping task's entries in the queues.  */
struct __benaphore_waiter {
  struct __benaphore_waiter *next;
  struct Task *task;
  bool granted;
};

struct __benaphore_cond_waiter {
  struct __benaphore_cond_waiter *next;
  struct Task *task;
  bool signalled;
};

/* Locked, with nobody in the queue.  */
#define HELD(b) ((unsigned long) (b).__last == 1)

#define WAIT_MS 40

static long long
now_ms (void)
{
  struct timeval tv;

  gettimeofday (&tv, 0);
  return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

static benaphore_time_t
in_ms (long ms)
{
  long long t = now_ms () + ms;
  benaphore_time_t d;

  d.__sec = t / 1000;
  d.__nsec = (t % 1000) * 1000000;
  return d;
}

/* A wait must leave no signal behind for a later Wait().  */
static void
check_no_signal (void)
{
  if (SetSignal (0, 0) & SIGF_SINGLE)
    abort ();
}

int
main (void)
{
  static benaphore_t b;
  static benaphore_recursive_t r;
  static benaphore_cond_t c = BENAPHORE_COND_INITIALIZER;
  struct __benaphore_waiter w, w2;
  struct __benaphore_cond_waiter cw;
  benaphore_time_t d;
  long long start;

  /* A free lock is taken, whatever the deadline.  */
  d = in_ms (-1000);
  if (benaphore_timedlock (&b, &d) != 0 || !HELD (b))
    abort ();

  /* Held, past its deadline: no wait at all.  */
  if (benaphore_timedlock (&b, &d) != ETIMEDOUT || !HELD (b))
    abort ();

  /* Held: waits for the deadline, and leaves the queue as it was.  */
  start = now_ms ();
  d = in_ms (WAIT_MS);
  if (benaphore_timedlock (&b, &d) != ETIMEDOUT)
    abort ();
  if (now_ms () - start < WAIT_MS - 20 || !HELD (b))
    abort ();
  check_no_signal ();

  /* The same with two sleepers queued: this task goes in last and comes
     off again, and w2 is the last one once more.  */
  w.task = w2.task = FindTask (0);
  w.granted = w2.granted = false;
  w.next = &w2;
  w2.next = &w;
  b.__last = (struct __benaphore_waiter *) ((unsigned long) &w2 | 1);
  d = in_ms (WAIT_MS);
  if (benaphore_timedlock (&b, &d) != ETIMEDOUT)
    abort ();
  if ((unsigned long) b.__last != ((unsigned long) &w2 | 1)
      || w.next != &w2 || w2.next != &w || w.granted || w2.granted)
    abort ();
  check_no_signal ();
  b.__last = (struct __benaphore_waiter *) 1;
  benaphore_unlock (&b);
  if (b.__last)
    abort ();

  /* recursive: the owner counts, without a timer */
  d = in_ms (-1000);
  if (benaphore_recursive_timedlock (&r, &d) != 0
      || benaphore_recursive_timedlock (&r, &d) != 0 || r.__count != 2)
    abort ();

  /* A wait gives the lock up whatever the count, and restores it.  */
  d = in_ms (WAIT_MS);
  if (benaphore_cond_timedwait_recursive (&c, &r, &d) != ETIMEDOUT)
    abort ();
  if (r.__count != 2 || r.__owner != FindTask (0) || !HELD (r.__b)
      || c.__head || c.__tail)
    abort ();
  benaphore_recursive_unlock (&r);
  benaphore_recursive_unlock (&r);
  if (r.__b.__last || r.__owner || r.__count)
    abort ();
  check_no_signal ();

  /* Past its deadline, a condition wait keeps the lock throughout.  */
  benaphore_lock (&b);
  d = in_ms (-1000);
  if (benaphore_cond_timedwait (&c, &b, &d) != ETIMEDOUT || !HELD (b)
      || c.__head)
    abort ();

  /* It gives the lock up, comes off the queue and takes the lock back,
     behind a waiter that stays queued.  */
  cw.next = 0;
  cw.task = FindTask (0);
  cw.signalled = false;
  c.__head = c.__tail = &cw;
  start = now_ms ();
  d = in_ms (WAIT_MS);
  if (benaphore_cond_timedwait (&c, &b, &d) != ETIMEDOUT)
    abort ();
  if (now_ms () - start < WAIT_MS - 20 || !HELD (b))
    abort ();
  if (c.__head != &cw || c.__tail != &cw || cw.next)
    abort ();
  check_no_signal ();

  /* signal and broadcast wake that waiter once, off the queue.  */
  benaphore_cond_signal (&c);
  if (!cw.signalled || c.__head || c.__tail)
    abort ();
  SetSignal (0, SIGF_SINGLE);
  benaphore_cond_signal (&c);
  check_no_signal ();

  cw.signalled = false;
  c.__head = c.__tail = &cw;
  benaphore_cond_broadcast (&c);
  if (!cw.signalled || c.__head || c.__tail)
    abort ();
  SetSignal (0, SIGF_SINGLE);
  benaphore_unlock (&b);

  return 0;
}
