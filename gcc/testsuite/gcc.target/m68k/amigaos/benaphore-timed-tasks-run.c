/* { dg-do run } */
/* { dg-skip-if "CreateNewProc() is Kickstart 2.0" { *-*-* } { "-mcrt=nix13" } } */
/* { dg-skip-if "ixemul faults in a process it did not create, AmigaPorts/m68k-amigaos-gcc#353" { *-*-* } { "-mcrt=ixemul" } } */
/* Timed waits of <benaphore.h> ended by another task with the timer still
   pending: a condition signalled, a lock handed over.  None may time out or
   leave SIGF_SINGLE behind.  The tasks sleep on a timed wait, not Delay():
   the simulator switches tasks only in Wait().  */
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <dos/dostags.h>
#include <exec/tasks.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <benaphore.h>

#define ROUNDS 20

static benaphore_t m, l, nap_lock;
static benaphore_cond_t c, never;
static struct Task *parent;
static volatile int turn, go, done, finished, bad;

static benaphore_time_t
in_us (long us)
{
  struct timeval tv;
  benaphore_time_t d;

  gettimeofday (&tv, 0);
  us += tv.tv_usec;
  d.__sec = tv.tv_sec + us / 1000000;
  d.__nsec = us % 1000000 * 1000;
  return d;
}

/* Gives the CPU to the other task.  */
static void
nap (void)
{
  benaphore_time_t d = in_us (2000);

  benaphore_lock (&nap_lock);
  benaphore_cond_timedwait (&never, &nap_lock, &d);
  benaphore_unlock (&nap_lock);
}

static void
check_no_signal (void)
{
  if (SetSignal (0, 0) & SIGF_SINGLE)
    bad++;
}

static void
pingpong (int me)
{
  benaphore_time_t d;
  int i;

  for (i = 0; i < ROUNDS; i++)
    {
      benaphore_lock (&m);
      while (turn != me)
	{
	  d = in_us (10000000);
	  if (benaphore_cond_timedwait (&c, &m, &d))
	    bad++;
	}
      check_no_signal ();
      turn = !me;
      benaphore_cond_signal (&c);
      benaphore_unlock (&m);
    }
}

/* A new process starts without the program's a4.  */
#ifdef __baserel__
#define SAVEDS __saveds
#else
#define SAVEDS
#endif

static void SAVEDS
worker (void)
{
  benaphore_time_t d;
  int i;

  pingpong (1);
  for (i = 0; i < ROUNDS; i++)
    {
      while (!go)
	nap ();
      go = 0;
      d = in_us (10000000);
      if (benaphore_timedlock (&l, &d))
	bad++;
      check_no_signal ();
      benaphore_unlock (&l);
      done = i + 1;
    }
  /* The code stays until the task is gone.  */
  Forbid ();
  finished = 1;
  Signal (parent, SIGF_SINGLE);
}

int
main (void)
{
  int i;

  parent = FindTask (0);
  if (!CreateNewProcTags (NP_Entry, (ULONG) worker, NP_Name, (ULONG) "worker",
			  TAG_END))
    abort ();
  pingpong (0);
  for (i = 0; i < ROUNDS; i++)
    {
      benaphore_lock (&l);
      go = 1;
      while (!((unsigned long) __atomic_load_n (&l.__last, __ATOMIC_RELAXED)
	       & ~1UL))
	nap ();
      benaphore_unlock (&l);
      while (done != i + 1)
	nap ();
    }
  while (!finished)
    nap ();
  SetSignal (0, SIGF_SINGLE);
  if (bad)
    abort ();
  return 0;
}
