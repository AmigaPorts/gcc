/* { dg-do run } */
/* { dg-require-effective-target amigaos_gthreads } */
/* Three threads in the C library's usleep() at once.
   gthread-usleep-newlib-run.c runs it with newlib.  */
#include <stdio.h>
#include <unistd.h>
#include "../../../../../libgcc/gthr-amigaos.h"

static void *sleeper(void *arg)
{
	int i;

	for (i = 0; i < 5; i++)
		usleep(20000);
	return arg;
}

int main(void)
{
	__gthread_t t[3];
	int i;

	for (i = 0; i < 3; i++)
		if (__gthread_create(&t[i], sleeper, 0))
			return 1;
	for (i = 0; i < 3; i++)
		__gthread_join(t[i], 0);
	printf("usleep from 3 threads: ok\n");
	return 0;
}
