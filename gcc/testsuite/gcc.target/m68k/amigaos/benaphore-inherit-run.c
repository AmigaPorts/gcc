/* { dg-do run } */
/* { dg-skip-if "CreateNewProc() is Kickstart 2.0" { *-*-* } { "-mcrt=nix13" } } */
/* Priority inheritance of the recursive benaphore.  A task of low priority
   holds the lock, and a busy task of middle priority is ready, when the
   main task, of high priority, sleeps on the lock.  exec runs the highest
   ready task, so the owner can release the lock before the busy task ends
   only if the sleeper has raised it above the busy one; and it has its own
   priority back once it has.  */
#include <stdlib.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/tasks.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <benaphore.h>

#define HIGH 2
#define MIDDLE 0
#define LOW -1
#define SPINS 1000000

static benaphore_recursive_t r;
static struct Task *main_task, *low_task;
static ULONG done;			/* main's signal for the others */
static volatile long spins;
static volatile int got_lock, ended;
static volatile BYTE pri_holding, pri_after;

static void
low (void)
{
  low_task = FindTask (0);
  benaphore_recursive_lock (&r);
  Signal (main_task, done);
  Wait (SIGBREAKF_CTRL_F);
  pri_holding = low_task->tc_Node.ln_Pri;
  benaphore_recursive_unlock (&r);
  pri_after = low_task->tc_Node.ln_Pri;
  Forbid ();
  ended++;
  Signal (main_task, done);
}

static void
busy (void)
{
  for (spins = 0; spins < SPINS && !got_lock; spins++)
    ;
  Forbid ();
  ended++;
  Signal (main_task, done);
}

static void
start (void (*entry) (void), const char *name, LONG pri)
{
  if (!CreateNewProcTags (NP_Entry, (ULONG) entry, NP_Name, (ULONG) name,
			  NP_Priority, pri, TAG_END))
    abort ();
}

int
main (void)
{
  long spins_then;
  BYTE sig;

  main_task = FindTask (0);
  SetTaskPri (main_task, HIGH);
  if ((sig = AllocSignal (-1)) == -1)
    abort ();
  done = 1UL << sig;

  start (low, "low", LOW);
  Wait (done);				/* low holds the lock */
  ended = 0;
  start (busy, "busy", MIDDLE);		/* ready, below us */
  Signal (low_task, SIGBREAKF_CTRL_F);	/* ready, below busy */

  benaphore_recursive_lock (&r);
  spins_then = spins;
  got_lock = 1;
  benaphore_recursive_unlock (&r);

  while (ended < 2)			/* busy and low */
    Wait (done);
  if (spins_then >= SPINS)
    abort ();				/* busy ran to its end first */
  if (pri_holding != HIGH || pri_after != LOW)
    abort ();
  return 0;
}
