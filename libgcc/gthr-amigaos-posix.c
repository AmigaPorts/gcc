/**
 * This is the posix.libraray-based implementation of gcc threads abstraction.
 */

#include "gthr-amigaos.h"

#include <pthread.h>
#include <proto/exec.h>

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************/

typedef struct
{
  union
  {
    __gthread_once_t gonce;
    pthread_once_t ponce;
  } u;
} __internal_gthread_once_t;

typedef struct
{
  union
  {
    __gthread_mutex_t gmutex;
    pthread_mutex_t pmutex;
  } u;
} __internal_gthread_mutex_t;

typedef struct
{
  union
  {
    __gthread_cond_t gcond;
    pthread_cond_t pcond;
  } u;
} __internal_gthread_cond_t;


/******************************************************************************/

#include "gthr-amigaos-asserts.h"

/******************************************************************************/

int
__gthread_active_p (void)
{
  /* Thread-system is always active as we have to be explicitly linked to the
   * final binary.
   */
  return 1;
}

int
__gthread_once (__gthread_once_t *__once, void (*__func) (void))
{
  __internal_gthread_once_t *once = (__internal_gthread_once_t *)__once;

  if (__gthread_active_p ())
    return pthread_once (&once->u.ponce, __func);
  else
    return -1;
}

int
__gthread_key_create (__gthread_key_t *__key, void (*__func) (void *))
{
  int err;
  pthread_key_t key;

  if ((err = pthread_key_create (&key, __func)))
    return err;
  __key->id = key;
  return 0;
}

int
__gthread_key_delete (__gthread_key_t __key)
{
  return pthread_key_delete (__key.id);
}

void *
__gthread_getspecific (__gthread_key_t __key)
{
  return pthread_getspecific (__key.id);
}

int
__gthread_setspecific (__gthread_key_t __key, const void *__v)
{
  return pthread_setspecific (__key.id, __v);
}

int
__gthread_mutex_init (__gthread_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;

  return pthread_mutex_init (&mx->u.pmutex, 0);
}

int
__gthread_mutex_destroy (__gthread_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;

  return pthread_mutex_destroy (&mx->u.pmutex);
}

int
__gthread_mutex_lock (__gthread_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;

  return pthread_mutex_lock (&mx->u.pmutex);
}

int
__gthread_mutex_trylock (__gthread_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;

  return pthread_mutex_trylock (&mx->u.pmutex);
}

int
__gthread_mutex_unlock (__gthread_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;

  return pthread_mutex_unlock (&mx->u.pmutex);
}

int
__gthread_recursive_mutex_init (__gthread_recursive_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;

  pthread_mutexattr_t attr;
  int err;

  if ((err = pthread_mutexattr_init (&attr)))
    return err;
  if ((err = pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE)))
    goto bailout;
  if ((err = pthread_mutex_init (&mx->u.pmutex, &attr)))
    goto bailout;
  return pthread_mutexattr_destroy (&attr);
bailout:
  pthread_mutexattr_destroy (&attr);
  return err;
}

int
__gthread_recursive_mutex_lock (__gthread_recursive_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;
  return pthread_mutex_lock (&mx->u.pmutex);
}

int
__gthread_recursive_mutex_trylock (__gthread_recursive_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;
  return pthread_mutex_trylock (&mx->u.pmutex);
}

int
__gthread_recursive_mutex_unlock (__gthread_recursive_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;
  return pthread_mutex_unlock (&mx->u.pmutex);
}

int
__gthread_recursive_mutex_destroy (__gthread_recursive_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;
  return pthread_mutex_destroy (&mx->u.pmutex);
}

int
__gthread_create (__gthread_t *thread, void *(*func) (void*), void *args)
{
  int err;
  pthread_t pthread;

  if ((err =  pthread_create (&pthread, NULL, func, args)))
    return err;
  *thread = pthread;
  return 0;
}

int
__gthread_join (__gthread_t thread, void **value_ptr)
{
  return pthread_join (thread, value_ptr);
}

int
__gthread_detach (__gthread_t thread)
{
  return pthread_detach (thread);
}

int
__gthread_equal (__gthread_t t1, __gthread_t t2)
{
  return pthread_equal (t1, t2);
}

__gthread_t __gthread_self (void)
{
  return pthread_self ();
}

int
__gthread_yield (void)
{
  IExec->Reschedule();
  return 0;
}

#ifdef __cplusplus
}
#endif