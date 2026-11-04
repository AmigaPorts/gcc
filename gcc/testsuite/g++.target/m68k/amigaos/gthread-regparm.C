// { dg-do compile }
// { dg-options "-O2 -fomit-frame-pointer -m68000 -mregparm=2 -mmangle-regparm" }

// libgcc's out-of-line gthread functions are built once, so the amigaos
// thread model's header fixes their convention whatever -mregparm and
// -mmangle-regparm the caller uses: the arguments on the stack, and the
// plain names.  With another thread model there is nothing to check here.

#include <bits/gthr.h>

__gthread_cond_t c = __GTHREAD_COND_INIT;
__gthread_mutex_t m = __GTHREAD_MUTEX_INIT;

extern "C" int
wait_it (void)
{
  return __gthread_cond_wait (&c, &m);
}

// { dg-final { scan-assembler-not "lea _c,a0" } }
// { dg-final { scan-assembler-not "@__gthread" } }
