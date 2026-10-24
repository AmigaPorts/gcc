/**
 * This is the single-thread implementation of gcc threads abstraction.
 */

#include "gthr-amigaos.h"

#include <errno.h>
#include <stddef.h>

#define UNUSED __attribute__((__unused__))

#ifdef __cplusplus
extern "C"
{
#endif

int
__gthread_active_p (void)
{
  return 0;
}

/* __GTHREADS is defined for every amigaos thread model, so libstdc++'s
   std::call_once relies on this to run the function.  data1 is the done
   flag, data2 set while the function runs.  */
int
__gthread_once (__gthread_once_t *__once, void (*__func) (void))
{
  if (__once == NULL || __func == NULL)
    return EINVAL;
  if (!__once->data1 && !__once->data2)
    {
      __once->data2 = 1;
      __func ();
      __once->data1 = 1;
    }
  return 0;
}

void
__gthread_once_unlock (__gthread_once_t *__once)
{
  __once->data2 = 0;
}

int UNUSED
__gthread_key_create (__gthread_key_t *__key UNUSED, void (*__func) (void *) UNUSED)
{
  return 0;
}

int UNUSED
__gthread_key_delete (__gthread_key_t __key UNUSED)
{
  return 0;
}

void *
__gthread_getspecific (__gthread_key_t __key UNUSED)
{
  return 0;
}

int
__gthread_setspecific (__gthread_key_t __key UNUSED, const void *__v UNUSED)
{
  return 0;
}

int
__gthread_mutex_init (__gthread_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_mutex_destroy (__gthread_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_mutex_lock (__gthread_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_mutex_trylock (__gthread_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_mutex_unlock (__gthread_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_recursive_mutex_init (__gthread_recursive_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_recursive_mutex_lock (__gthread_recursive_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_recursive_mutex_trylock (__gthread_recursive_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_recursive_mutex_unlock (__gthread_recursive_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_recursive_mutex_destroy (__gthread_recursive_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_create (__gthread_t *thread, void *(*func) (void*), void *args UNUSED)
{
  return 0;
}

int
__gthread_join (__gthread_t thread, void **value_ptr)
{
  return 0;
}

int
__gthread_detach (__gthread_t thread)
{
  return 0;
}

int
__gthread_equal (__gthread_t t1, __gthread_t t2)
{
  return 0;
}

__gthread_t __gthread_self (void)
{
  return 0;
}

int
__gthread_yield (void)
{
  return 0;
}

int
__gthread_mutex_timedlock (__gthread_mutex_t *__mutex UNUSED,
			   const __gthread_time_t *abs_timeout UNUSED)
{
  return 0;
}

int
__gthread_recursive_mutex_timedlock (__gthread_recursive_mutex_t *__mutex UNUSED,
				     const __gthread_time_t *abs_timeout UNUSED)
{
  return 0;
}

/* With a single thread nobody can signal a condition, so a wait returns at
   once, which callers must already handle as a spurious wakeup.  */
int
__gthread_cond_init (__gthread_cond_t *__cond UNUSED)
{
  return 0;
}

int
__gthread_cond_signal (__gthread_cond_t *__cond UNUSED)
{
  return 0;
}

int
__gthread_cond_broadcast (__gthread_cond_t *__cond UNUSED)
{
  return 0;
}

int
__gthread_cond_wait (__gthread_cond_t *__cond UNUSED,
		     __gthread_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_cond_wait_recursive (__gthread_cond_t *__cond UNUSED,
			       __gthread_recursive_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_cond_timedwait (__gthread_cond_t *__cond UNUSED,
			  __gthread_mutex_t *__mutex UNUSED,
			  const __gthread_time_t *abs_timeout UNUSED)
{
  return 0;
}

int
__gthread_cond_destroy (__gthread_cond_t *__cond UNUSED)
{
  return 0;
}

#ifdef __cplusplus
}
#endif
