/* Thread support for AmigaOS.
You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

#ifndef GCC_GTHR_AMIGAOS_H
#define GCC_GTHR_AMIGAOS_H

#ifdef _LIBOBJC
#include "gthr-single.h"
#elif defined (__amigaos3__)

/* AmigaOS 3.x on m68k: threads are DOS processes, and the locks are
   <benaphore.h>, whose uncontended paths are a few inline instructions and
   need no exec call.  All the types are valid when zeroed, so mutexes,
   once flags and condition variables can be static and std::mutex keeps
   its constexpr constructor.  The rest is in libgcc's gthr-amigaos*.c.

   Only one CPU, and only tasks, not interrupt code.  Threads should be
   started with __gthread_create() (std::thread): their keys are cleaned up
   when they end.  A task started another way gets its keys on first use,
   but nothing frees them or runs their destructors when it ends.  */

#include <benaphore.h>

/* External linkage in C++, as gthr-posix.h, for libstdc++'s modules.  */
#define __GTHREAD_INLINE __BENAPHORE_INLINE

#define __GTHREADS 1
#define __GTHREADS_CXX0X 1
/* std::call_once unlocks the flag when its function throws.  */
#define __GTHREAD_HAS_ONCE_UNLOCK 1

typedef benaphore_t __gthread_mutex_t;
typedef benaphore_recursive_t __gthread_recursive_mutex_t;

typedef struct {
  volatile unsigned char __done;
  benaphore_t __lock;
} __gthread_once_t;

struct __gthread_cond_waiter;
typedef struct {
  struct __gthread_cond_waiter *__head, *__tail;
} __gthread_cond_t;

typedef unsigned int __gthread_key_t;

/* The thread's struct Task.  */
typedef void *__gthread_t;

/* libstdc++ fills it as { tv_sec, tv_nsec }, whatever time_t is.  */
typedef struct {
  long long __sec;
  long __nsec;
} __gthread_time_t;

#define __GTHREAD_MUTEX_INIT BENAPHORE_INITIALIZER
#define __GTHREAD_MUTEX_INIT_FUNCTION(__mx) __gthread_mutex_init (__mx)
#define __GTHREAD_RECURSIVE_MUTEX_INIT BENAPHORE_RECURSIVE_INITIALIZER
#define __GTHREAD_ONCE_INIT { 0, BENAPHORE_INITIALIZER }
#define __GTHREAD_COND_INIT { 0, 0 }
#define __GTHREAD_HAS_COND 1

#ifdef __cplusplus
extern "C" {
#endif

/* Constant: emutls and libstdc++ pick their data layout from it once.  */
__GTHREAD_INLINE int
__gthread_active_p (void)
{
  return 1;
}

__GTHREAD_INLINE int
__gthread_once (__gthread_once_t *__once, void (*__func) (void))
{
  if (__builtin_expect (__once->__done, 1))
    {
      __asm__ __volatile__ ("" : : : "memory");
      return 0;
    }
  benaphore_lock (&__once->__lock);
  if (!__once->__done)
    {
      __func ();
      __once->__done = 1;
    }
  benaphore_unlock (&__once->__lock);
  return 0;
}

/* For std::call_once: __func threw, so let the next caller run it.  */
__GTHREAD_INLINE void
__gthread_once_unlock (__gthread_once_t *__once)
{
  benaphore_unlock (&__once->__lock);
}

__GTHREAD_INLINE int
__gthread_mutex_init (__gthread_mutex_t *__mx)
{
  __mx->__last = 0;
  return 0;
}

__GTHREAD_INLINE int
__gthread_mutex_destroy (__gthread_mutex_t *__mx __attribute__ ((__unused__)))
{
  return 0;
}

__GTHREAD_INLINE int
__gthread_mutex_lock (__gthread_mutex_t *__mx)
{
  benaphore_lock (__mx);
  return 0;
}

__GTHREAD_INLINE int
__gthread_mutex_trylock (__gthread_mutex_t *__mx)
{
  return benaphore_trylock (__mx) ? 0 : 16;	/* EBUSY */
}

__GTHREAD_INLINE int
__gthread_mutex_unlock (__gthread_mutex_t *__mx)
{
  benaphore_unlock (__mx);
  return 0;
}

__GTHREAD_INLINE int
__gthread_recursive_mutex_init (__gthread_recursive_mutex_t *__mx)
{
  __gthread_mutex_init (&__mx->__b);
  __mx->__owner = 0;
  __mx->__count = 0;
  return 0;
}

__GTHREAD_INLINE int
__gthread_recursive_mutex_destroy (__gthread_recursive_mutex_t *__mx
				   __attribute__ ((__unused__)))
{
  return 0;
}

__GTHREAD_INLINE int
__gthread_recursive_mutex_lock (__gthread_recursive_mutex_t *__mx)
{
  benaphore_recursive_lock (__mx);
  return 0;
}

__GTHREAD_INLINE int
__gthread_recursive_mutex_trylock (__gthread_recursive_mutex_t *__mx)
{
  return benaphore_recursive_trylock (__mx) ? 0 : 16;	/* EBUSY */
}

__GTHREAD_INLINE int
__gthread_recursive_mutex_unlock (__gthread_recursive_mutex_t *__mx)
{
  benaphore_recursive_unlock (__mx);
  return 0;
}

__GTHREAD_INLINE __gthread_t
__gthread_self (void)
{
  return __BENAPHORE_THIS_TASK ();
}

__GTHREAD_INLINE int
__gthread_equal (__gthread_t __t1, __gthread_t __t2)
{
  return __t1 == __t2;
}

__GTHREAD_INLINE int
__gthread_cond_init (__gthread_cond_t *__cond)
{
  __cond->__head = __cond->__tail = 0;
  return 0;
}

__GTHREAD_INLINE int
__gthread_cond_destroy (__gthread_cond_t *__cond __attribute__ ((__unused__)))
{
  return 0;
}

/* libgcc builds these once, so they take their arguments on the stack and
   keep their plain names whatever -mregparm and -mmangle-regparm the
   caller uses.  None of them runs code of the caller's in its task, so none
   throws: C++ callers that can't throw then need no exception handling
   around them.  */
#define __GTHREAD_NOTHROW __attribute__ ((__stkparm__, __nothrow__))

int __gthread_key_create (__gthread_key_t *__key, void (*__destroy) (void *))
  __GTHREAD_NOTHROW;
int __gthread_key_delete (__gthread_key_t __key) __GTHREAD_NOTHROW;
void *__gthread_getspecific (__gthread_key_t __key) __GTHREAD_NOTHROW;
int __gthread_setspecific (__gthread_key_t __key, const void *__v)
  __GTHREAD_NOTHROW;

int __gthread_cond_signal (__gthread_cond_t *__cond) __GTHREAD_NOTHROW;
int __gthread_cond_broadcast (__gthread_cond_t *__cond) __GTHREAD_NOTHROW;
int __gthread_cond_wait (__gthread_cond_t *__cond, __gthread_mutex_t *__mx)
  __GTHREAD_NOTHROW;
int __gthread_cond_wait_recursive (__gthread_cond_t *__cond,
				   __gthread_recursive_mutex_t *__mx)
  __GTHREAD_NOTHROW;
int __gthread_cond_timedwait (__gthread_cond_t *__cond,
			      __gthread_mutex_t *__mx,
			      const __gthread_time_t *__abs_timeout)
  __GTHREAD_NOTHROW;

int __gthread_mutex_timedlock (__gthread_mutex_t *__mx,
			       const __gthread_time_t *__abs_timeout)
  __GTHREAD_NOTHROW;
int __gthread_recursive_mutex_timedlock (__gthread_recursive_mutex_t *__mx,
					 const __gthread_time_t *__abs_timeout)
  __GTHREAD_NOTHROW;

int __gthread_create (__gthread_t *__thr, void *(*__func) (void *),
		      void *__args) __GTHREAD_NOTHROW;
int __gthread_join (__gthread_t __thr, void **__value_ptr) __GTHREAD_NOTHROW;
int __gthread_detach (__gthread_t __thr) __GTHREAD_NOTHROW;
int __gthread_yield (void) __GTHREAD_NOTHROW;

#ifdef __cplusplus
}
#endif

#else

#ifndef GCC_TSYSTEM_H
#include <stdlib.h>
#endif
#include <stdint.h>

typedef struct { int data1, data2; } __gthread_once_t;

typedef struct { long unsigned int id; } __gthread_key_t;

/* Should cover at least as much data as the maximum of sizeof(struct SignalSemaphore)
 * and sizeof(pthread_mutex_t)
 */
typedef struct { char data[48]; } __gthread_mutex_t;
typedef struct { char data[48]; } __gthread_recursive_mutex_t;

/* Large enough for the native condition and clib4's pthread_cond_t
   (12 or 20 bytes, depending on the clib4 version).  */
typedef struct { char data[24]; } __gthread_cond_t;

typedef int __gthread_t;
typedef struct { int seconds; long nanoseconds;} __gthread_time_t;

#define __GTHREADS 1
#define __GTHREADS_CXX0X 1
/* std::call_once unlocks the flag when its function throws.  */
#define __GTHREAD_HAS_ONCE_UNLOCK 1

#define __GTHREAD_ONCE_INIT {0,0}
//#define __GTHREAD_MUTEX_INIT 0
#define __GTHREAD_MUTEX_INIT_FUNCTION(mx) __gthread_mutex_init(mx)
//#define __GTHREAD_RECURSIVE_MUTEX_INIT 0
#define __GTHREAD_RECURSIVE_MUTEX_INIT_FUNCTION(mx) __gthread_recursive_mutex_init(mx)

//#define __GTHREAD_COND_INIT 0
#define __GTHREAD_COND_INIT_FUNCTION(cond) __gthread_cond_init(cond)

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _LIBOBJC
 
#ifndef UNUSED
#define UNUSED __attribute__((__unused__))
#define UNUSED_DEFINED
#endif

/* Thread local storage for a single thread */
static void *thread_local_storage = NULL;

/* Backend initialization functions */

/* Wake up one thread waiting on this condition.  */
static inline int
__gthread_objc_condition_signal (objc_condition_t condition UNUSED)
{
  return 0;
}
 
#ifndef UNUSED_DEFINED
#undef UNUSED
#endif

#else /* _LIBOBJC */

int
__gthread_active_p (void);

int
__gthread_once (__gthread_once_t *__once, void (*__func) (void));

void
__gthread_once_unlock (__gthread_once_t *__once);

int
__gthread_key_create (__gthread_key_t *__key, void (*destroy) (void *));

int
__gthread_key_delete (__gthread_key_t __key);

void *
__gthread_getspecific (__gthread_key_t __key);

int
__gthread_setspecific (__gthread_key_t __key, const void *__v);

int
__gthread_mutex_init (__gthread_mutex_t *__mutex);

int
__gthread_mutex_destroy (__gthread_mutex_t *__mutex);

int
__gthread_mutex_lock (__gthread_mutex_t *__mutex);

int
__gthread_mutex_trylock (__gthread_mutex_t *__mutex);

int
__gthread_mutex_unlock (__gthread_mutex_t *__mutex);

int
__gthread_recursive_mutex_init (__gthread_recursive_mutex_t *__mutex);

int
__gthread_recursive_mutex_lock (__gthread_recursive_mutex_t *__mutex);

int
__gthread_recursive_mutex_trylock (__gthread_recursive_mutex_t *__mutex);

int
__gthread_recursive_mutex_unlock (__gthread_recursive_mutex_t *__mutex);

int
__gthread_recursive_mutex_destroy (__gthread_recursive_mutex_t *__mutex);

/******************************************************************************/

int
__gthread_cond_init (__gthread_cond_t *cond);

int
__gthread_cond_signal (__gthread_cond_t *__cond);

int
__gthread_cond_broadcast (__gthread_cond_t *cond);

int
__gthread_cond_wait (__gthread_cond_t *cond, __gthread_mutex_t *mutex);

int
__gthread_cond_wait_recursive (__gthread_cond_t *cond,
                               __gthread_recursive_mutex_t *mutex);

int
__gthread_cond_timedwait (__gthread_cond_t *cond,
                            __gthread_mutex_t *mutex,
                            const __gthread_time_t *abs_timeout);

int
__gthread_cond_destroy (__gthread_cond_t *__cond);

/******************************************************************************/

int
__gthread_create (__gthread_t *thread, void *(*func) (void*), void *args);

int
__gthread_join (__gthread_t thread, void **value_ptr);

int
__gthread_detach (__gthread_t thread);

int
__gthread_equal (__gthread_t t1, __gthread_t t2);

__gthread_t
__gthread_self (void);

int
__gthread_yield (void);

int
__gthread_mutex_timedlock (__gthread_mutex_t *m, const __gthread_time_t *abs_timeout);

int
__gthread_recursive_mutex_timedlock (__gthread_recursive_mutex_t *m,
                                    const __gthread_time_t *abs_time);

#endif /* _LIBOBJC */

#ifdef __cplusplus
}
#endif

#endif /* _LIBOBJC */

#endif /* ! GCC_GTHR_AMIGAOS_H */
