/* { dg-do run } */
/* { dg-require-effective-target amigaos_gthreads } */
/* A newlib program built without -pthread has newlib's locks that do
   nothing and one reent for every task, so libgcc refuses to start a
   thread in it, with ENOSYS.  Not named gthread-*, which amigaos.exp builds
   with -pthread when they run with newlib.  */

#include <errno.h>
#include "../../../../../libgcc/gthr-amigaos.h"

static int ran;

static void *thread(void *arg)
{
	ran = 1;
	return arg;
}

int main(void)
{
	__gthread_t t;
	int err = __gthread_create(&t, thread, 0);

	/* A thread that started anyway ends before the program does.  */
	if (err == 0)
		__gthread_join(t, 0);
	return err != ENOSYS || ran;
}
