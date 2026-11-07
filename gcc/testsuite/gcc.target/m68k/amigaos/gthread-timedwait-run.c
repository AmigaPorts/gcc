/* { dg-do run } */
/* { dg-require-effective-target amigaos_gthreads } */
/* A timed condition wait that nobody signals must time out: with no other
   task, and again after a thread has started and ended.  */
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <proto/dos.h>
#include "../../../../../libgcc/gthr-amigaos.h"

static __gthread_cond_t cond = __GTHREAD_COND_INIT;
static __gthread_mutex_t mx = __GTHREAD_MUTEX_INIT;

static void *quick(void *arg)
{
	Delay(2);
	return arg;
}

static int times_out(void)
{
	__gthread_time_t when;
	struct timeval tv;
	int ret;

	gettimeofday(&tv, 0);
	when.__sec = tv.tv_sec + 1;
	when.__nsec = tv.tv_usec * 1000L;
	__gthread_mutex_lock(&mx);
	ret = __gthread_cond_timedwait(&cond, &mx, &when);
	__gthread_mutex_unlock(&mx);
	printf("timedwait: %s\n", ret == ETIMEDOUT ? "ETIMEDOUT" : "other");
	return ret == ETIMEDOUT;
}

int main(void)
{
	__gthread_t t;

	if (!times_out())
		return 1;
	if (__gthread_create(&t, quick, 0))
		return 2;
	__gthread_join(t, 0);
	if (!times_out())
		return 3;
	return 0;
}
