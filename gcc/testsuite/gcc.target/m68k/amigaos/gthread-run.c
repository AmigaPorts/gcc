/* { dg-do run } */
/* { dg-require-effective-target amigaos_gthreads } */
/* gthr-amigaos with several tasks: once, mutexes, keys and their
   destructors, condition variables with and without a timeout, join,
   detach, the recursive mutex, timed locks, and memory over many short
   threads.  Prints what fails; the exit code is the number of failures.
   Only main prints: the C library's stdio isn't safe across tasks.
   gthread-newlib-run.c runs it with newlib.  */

#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "../../../../../libgcc/gthr-amigaos.h"

extern struct ExecBase *SysBase;

static int failures;

static void check(int ok, const char *what)
{
	if (!ok) {
		printf("FAIL: %s\n", what);
		failures++;
	}
}

/* In milliseconds: no floating point, which libnix links only with -lm.  */
static long long now(void)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

static __gthread_time_t after_ms(int ms)
{
	struct timeval tv;
	__gthread_time_t t;

	gettimeofday(&tv, 0);
	t.__sec = tv.tv_sec + (tv.tv_usec + ms * 1000L) / 1000000;
	t.__nsec = (tv.tv_usec + ms * 1000L) % 1000000 * 1000;
	return t;
}

/* 1. once, from four threads at a time */
static __gthread_once_t once = __GTHREAD_ONCE_INIT;
static volatile int once_runs;

static void once_func(void)
{
	Delay(5);	/* the others queue behind it */
	once_runs++;
}

static void *once_thread(void *arg)
{
	__gthread_once(&once, once_func);
	return (void *) (long) (once_runs == 1) + (long) arg * 0;
}

/* 2. a mutex, sleeping inside it now and then */
static __gthread_mutex_t mx = __GTHREAD_MUTEX_INIT;
static volatile long counter;
static volatile int inside, max_inside;

static void *counter_thread(void *arg)
{
	int i;
	for (i = 0; i < 300; i++) {
		__gthread_mutex_lock(&mx);
		if (++inside > max_inside)
			max_inside = inside;
		counter++;
		if (i % 50 == (long) arg)
			Delay(1);
		inside--;
		__gthread_mutex_unlock(&mx);
	}
	return 0;
}

/* 3. keys: each thread sees its own value, and the destructor runs */
static __gthread_key_t key;
static volatile int destroyed, key_ok;

static void key_destroy(void *v)
{
	(void) v;
	Forbid();
	destroyed++;
	Permit();
}

static void *key_thread(void *arg)
{
	int ok;
	ok = __gthread_getspecific(key) == 0;
	__gthread_setspecific(key, arg);
	Delay(2);	/* the others set theirs meanwhile */
	ok = ok && __gthread_getspecific(key) == arg;
	Forbid();
	key_ok += ok;
	Permit();
	return 0;
}

/* 4. condition variables: a queue of items, and a broadcast */
static __gthread_cond_t cond = __GTHREAD_COND_INIT;
static volatile int items, consumed, go, woken;

static void *consumer(void *arg)
{
	(void) arg;
	for (;;) {
		__gthread_mutex_lock(&mx);
		while (!items && consumed < 100)
			__gthread_cond_wait(&cond, &mx);
		if (consumed >= 100) {
			__gthread_mutex_unlock(&mx);
			return 0;
		}
		items--;
		consumed++;
		if (consumed == 100)
			__gthread_cond_broadcast(&cond);
		__gthread_mutex_unlock(&mx);
	}
}

static void *go_waiter(void *arg)
{
	(void) arg;
	__gthread_mutex_lock(&mx);
	while (!go)
		__gthread_cond_wait(&cond, &mx);
	woken++;
	__gthread_mutex_unlock(&mx);
	return 0;
}

/* 6. the recursive mutex, held twice across a wait */
static __gthread_recursive_mutex_t rmx = __GTHREAD_RECURSIVE_MUTEX_INIT;
static volatile int rflag;

static void *recursive_waiter(void *arg)
{
	int ok;
	(void) arg;
	__gthread_recursive_mutex_lock(&rmx);
	__gthread_recursive_mutex_lock(&rmx);
	while (!rflag)
		__gthread_cond_wait_recursive(&cond, &rmx);
	ok = rmx.__count == 2 && rmx.__owner == __gthread_self();
	__gthread_recursive_mutex_unlock(&rmx);
	__gthread_recursive_mutex_unlock(&rmx);
	return (void *) (long) ok;
}

/* 7. a thread that holds the mutex until it is told to let go.  It waits
   on a condition rather than for a time, so the test doesn't depend on
   how the tasks are scheduled.  */
static __gthread_mutex_t hold_mx = __GTHREAD_MUTEX_INIT;
static __gthread_cond_t hold_cond = __GTHREAD_COND_INIT;
static int holding, release;

static void *holder(void *arg)
{
	__gthread_mutex_lock(&mx);
	__gthread_mutex_lock(&hold_mx);
	holding = 1;
	__gthread_cond_broadcast(&hold_cond);
	while (!release)
		__gthread_cond_wait(&hold_cond, &hold_mx);
	__gthread_mutex_unlock(&hold_mx);
	__gthread_mutex_unlock(&mx);
	return arg;
}

static void *nothing(void *arg)
{
	return arg;
}

/* 8. a thread that only says it ran, for the yield */
static volatile int yielded_to;

static void *mark(void *arg)
{
	yielded_to = 1;
	return arg;
}

/* A joined thread can still be in DOS's exit code, which frees its stack
   and process: let it run to its end before counting memory.  A yield runs
   the ready tasks of our priority, and the Delay() the rest.  */
static void settle(void)
{
	int i;

	for (i = 0; i < 20; i++)
		__gthread_yield();
	Delay(10);
}

static void *baserel_thread(void *arg)
{
	/* a global: with -fbaserel, only right if a4 is the creator's */
	return (void *) (long) (counter + (long) arg);
}

int main(void)
{
	__gthread_t t[8];
	void *r;
	int i, ok;
	long long t0;
	ULONG mem;

	/* 1 */
	for (i = 0; i < 4; i++)
		check(__gthread_create(&t[i], once_thread, (void *) (long) i) == 0, "create");
	for (ok = 1, i = 0; i < 4; i++) {
		check(__gthread_join(t[i], &r) == 0, "join");
		ok = ok && r == (void *) 1;
	}
	check(once_runs == 1 && ok, "once: run once, and every caller waited for it");
	printf("1. once: %d run\n", once_runs);

	/* 2 */
	for (i = 0; i < 4; i++)
		__gthread_create(&t[i], counter_thread, (void *) (long) (i * 7));
	for (i = 0; i < 4; i++)
		__gthread_join(t[i], 0);
	check(counter == 1200 && max_inside == 1, "mutex: count, one inside at a time");
	check(!mx.__last, "mutex: left unlocked");
	printf("2. mutex: count %ld\n", counter);

	/* 3 */
	check(__gthread_key_create(&key, key_destroy) == 0, "key_create");
	__gthread_setspecific(key, (void *) 99);
	for (i = 0; i < 4; i++)
		__gthread_create(&t[i], key_thread, (void *) (long) (i + 1));
	for (i = 0; i < 4; i++)
		__gthread_join(t[i], 0);
	check(key_ok == 4, "keys: each thread its own value");
	check(destroyed == 4, "keys: destructor once per thread");
	check(__gthread_getspecific(key) == (void *) 99, "keys: main keeps its value");
	__gthread_key_delete(key);
	check(__gthread_key_create(&key, 0) == 0 && __gthread_getspecific(key) == 0,
	      "keys: a new key starts out empty");
	printf("3. keys: %d ok, %d destroyed\n", key_ok, destroyed);

	/* 4 */
	for (i = 0; i < 3; i++)
		__gthread_create(&t[i], consumer, 0);
	for (i = 0; i < 100; i++) {
		__gthread_mutex_lock(&mx);
		items++;
		__gthread_cond_signal(&cond);
		__gthread_mutex_unlock(&mx);
		if (i % 10 == 0)
			Delay(1);
	}
	for (i = 0; i < 3; i++)
		__gthread_join(t[i], 0);
	check(consumed == 100 && items == 0, "cond: every item consumed once");
	check(!cond.__head, "cond: no waiter left");
	for (i = 0; i < 3; i++)
		__gthread_create(&t[i], go_waiter, 0);
	Delay(10);
	__gthread_mutex_lock(&mx);
	go = 1;
	__gthread_cond_broadcast(&cond);
	__gthread_mutex_unlock(&mx);
	for (i = 0; i < 3; i++)
		__gthread_join(t[i], 0);
	check(woken == 3, "cond: broadcast wakes all");
	printf("4. cond: %d consumed, %d woken\n", consumed, woken);

	/* 5. timeouts */
	{
		__gthread_time_t when = after_ms(300);
		int ret;

		__gthread_mutex_lock(&mx);
		t0 = now();
		ret = __gthread_cond_timedwait(&cond, &mx, &when);
		t0 = now() - t0;
		__gthread_mutex_unlock(&mx);
		check(ret == ETIMEDOUT, "timedwait: ETIMEDOUT");
		check(t0 > 250 && t0 < 600, "timedwait: about 0.3 s");
		check(!cond.__head, "timedwait: off the queue");
		printf("5. timedwait: %d after %ld ms\n", ret, (long) t0);

		__gthread_create(&t[0], holder, 0);
		__gthread_mutex_lock(&hold_mx);
		while (!holding)
			__gthread_cond_wait(&hold_cond, &hold_mx);
		__gthread_mutex_unlock(&hold_mx);
		when = after_ms(200);
		check(__gthread_mutex_timedlock(&mx, &when) == ETIMEDOUT, "timedlock: times out");
		__gthread_mutex_lock(&hold_mx);
		release = 1;
		__gthread_cond_broadcast(&hold_cond);
		__gthread_mutex_unlock(&hold_mx);
		when = after_ms(2000);
		check(__gthread_mutex_timedlock(&mx, &when) == 0, "timedlock: gets it");
		__gthread_mutex_unlock(&mx);
		__gthread_join(t[0], 0);

		/* a signal before the timeout */
		__gthread_create(&t[0], go_waiter, 0);
		Delay(5);
		__gthread_mutex_lock(&mx);
		go = 0;
		__gthread_mutex_unlock(&mx);
	}
	__gthread_mutex_lock(&mx);
	go = 1;
	__gthread_cond_broadcast(&cond);
	__gthread_mutex_unlock(&mx);
	__gthread_join(t[0], 0);

	/* 6 */
	__gthread_create(&t[0], recursive_waiter, 0);
	Delay(5);
	__gthread_recursive_mutex_lock(&rmx);
	rflag = 1;
	__gthread_cond_broadcast(&cond);
	__gthread_recursive_mutex_unlock(&rmx);
	__gthread_join(t[0], &r);
	check(r == (void *) 1, "recursive: count and owner back after the wait");
	check(!rmx.__b.__last && !rmx.__count, "recursive: left unlocked");

	/* join's result, detach, self, equal */
	__gthread_create(&t[0], nothing, (void *) 1234);
	__gthread_join(t[0], &r);
	check(r == (void *) 1234, "join: the thread's result");
	check(__gthread_join(t[0], &r) == ESRCH, "join: twice is ESRCH");
	__gthread_create(&t[0], nothing, 0);
	check(__gthread_detach(t[0]) == 0, "detach");
	__gthread_create(&t[1], nothing, 0);
	Delay(5);
	check(__gthread_detach(t[1]) == 0, "detach after it ended");
	check(__gthread_equal(__gthread_self(), SysBase->ThisTask), "self");
	check(__gthread_yield() == 0, "yield");
	yielded_to = 0;
	__gthread_create(&t[0], mark, 0);
	for (i = 0; i < 50 && !yielded_to; i++)
		__gthread_yield();
	check(yielded_to, "yield: a ready thread of the same priority runs");
	__gthread_join(t[0], 0);

	__gthread_create(&t[0], baserel_thread, (void *) 1L);
	__gthread_join(t[0], &r);
	check(r == (void *) (counter + 1), "a thread reads the creator's globals");

	/* 50 short threads: no memory lost */
	settle();
	mem = AvailMem(MEMF_ANY);
	for (i = 0; i < 50; i++) {
		__gthread_create(&t[0], key_thread, (void *) 7L);
		__gthread_join(t[0], 0);
	}
	settle();
	printf("7. 50 threads: %ld bytes\n", (long) mem - (long) AvailMem(MEMF_ANY));
	check((long) mem - (long) AvailMem(MEMF_ANY) < 1024, "50 threads: no memory lost");

	printf("%d failed\n", failures);
	return failures;
}
