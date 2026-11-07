/* { dg-do run } */
/* { dg-require-effective-target amigaos_gthreads } */
/* A joined thread at a lower priority than main: once the join returns,
   the thread must have none of the program's code left to run, or the
   program could end and be unloaded under it.  The thread lowers its own
   priority and ends; right after the join, main looks for it in exec's
   task lists.  Gone, or awake in DOS's exit code, is fine.  Still waiting
   to come back from the join handshake's Wait(SIGF_SINGLE), which it
   would do in the program's code, is the bug.  */

#include <stdio.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include "../../../../../libgcc/gthr-amigaos.h"

extern struct ExecBase *SysBase;

static void *lower(void *arg)
{
	SetTaskPri(SysBase->ThisTask, -1);
	return arg;
}

static int in_list(struct List *list, void *task)
{
	struct Node *n;

	for (n = list->lh_Head; n->ln_Succ; n = n->ln_Succ)
		if (n == task)
			return 1;
	return 0;
}

int main(void)
{
	__gthread_t t;
	int alive, pending;

	if (__gthread_create(&t, lower, 0))
		return 20;
	__gthread_join(t, 0);
	Disable();
	alive = in_list(&SysBase->TaskReady, t) || in_list(&SysBase->TaskWait, t);
	/* Only read while it is still a task: once gone it may be freed.  */
	pending = alive && (((struct Task *) t)->tc_SigRecvd & SIGF_SINGLE);
	Enable();
	printf("after join, the thread is %s\n",
	       !alive ? "gone" : pending ? "yet to wake in the program's code"
				     : "awake");
	return pending ? 10 : 0;
}
