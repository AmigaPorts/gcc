/* Thread support for AmigaOS.
You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

#ifndef GCC_GTHR_AMIGAOS_H
#define GCC_GTHR_AMIGAOS_H

#ifdef _LIBOBJC
#include "gthr-single.h"
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

typedef struct { char data[8]; } __gthread_cond_t;

typedef int __gthread_t;
typedef struct { int seconds; long nanoseconds;} __gthread_time_t;

#define __GTHREADS 1
#define __GTHREADS_CXX0X 1

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