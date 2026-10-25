#ifndef __cplusplus
_Static_assert(__atomic_always_lock_free(sizeof(uint32_t), 0), "Access to uint32_t is not lock free");
_Static_assert(__atomic_always_lock_free(sizeof(char), 0), "Access to char is not lock free");
_Static_assert(sizeof(__gthread_once_t) >= sizeof(__internal_gthread_once_t), "__gthread_once_t not large enough!");
_Static_assert(sizeof(__gthread_mutex_t) >= sizeof(__internal_gthread_mutex_t), "__gthread_mutex_t not large enough!");
_Static_assert(sizeof(__gthread_recursive_mutex_t) >= sizeof(__internal_gthread_mutex_t), "__gthread_recursive_mutex_t not large enough!");
_Static_assert(sizeof(__gthread_cond_t) >= sizeof(__internal_gthread_cond_t), "__gthread_recursive_mutex_t not large enough!");
#endif
