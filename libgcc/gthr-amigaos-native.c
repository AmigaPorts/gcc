/**
 * This is the native implementation of gcc threads abstraction. The advantage
 * over the pthreads one is that no pthreads.library is needed.
 *
 * (c) 2018 by Sebastian Bauer
 */
#define __NOLIBBASE__
#define __NOGLOBALIFACE__

#include "gthr-amigaos.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/timer.h>

/******************************************************************************/

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

/******************************************************************************/

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
    struct
    {
      char done;
      char started;
    } i;
  } u;
} __internal_gthread_once_t;

typedef struct
{
  union
  {
    __gthread_mutex_t gmutex;
    struct
    {
      struct SignalSemaphore sem;
      uint8_t recursive;
      uint8_t acquired;
    } i;
  } u;
} __internal_gthread_mutex_t;

typedef struct
{
  union
  {
    __gthread_cond_t gcond;
    struct
    {
      struct threadentry *first_in_cond_wait_list;
    } i;
  } u;
} __internal_gthread_cond_t;

/******************************************************************************/

#include "gthr-amigaos-asserts.h"

/******************************************************************************/

static struct ExecIFace *iexec = 0;
static struct DOSIFace *idos = 0;
static struct Library *DOSBase;

static __gthread_once_t libs_once = __GTHREAD_ONCE_INIT;

/**
 * The init function to open all necessary libs.
 */
static void init_libs(void)
{
  struct ExecBase *SysBase = *(struct ExecBase **)4;
  iexec = (struct ExecIFace *)SysBase->MainInterface;
  DOSBase = iexec->OpenLibrary("dos.library", 0);
  idos = (struct DOSIFace *)iexec->GetInterface(DOSBase, "main", 1, NULL);
}

/******************************************************************************/

/* We keep the entries of each key organized as a single linked list */
struct threadentry
{
  struct threadentry *next;

  struct Process *process;
  int id;

  struct MsgPort *timer_port;
  struct TimeRequest *timer_io;
  struct ITimer *itimer;

  /* The task that is going to join us */
  struct Task *joiner_task;

  /* The call to entry() has already been returned */
  int finished;

  /* Valid if finished is set */
  void *result;

  /* Thread is detached (and cannot be joined) */
  int detached;

  void *(*entry) (void*);
  void *args;

  /* The next thread in the condition wait list. A thread can only be in one
   * cond wait list so we can embed the linking in this structure.
   */
  struct threadentry *next_in_cond_wait_list;
};

typedef struct threadentry threadentry_t;

typedef struct
{
  struct SignalSemaphore sem;

  /* Singlely-linked list of threads */
  threadentry_t *threads;

  int next_thread_id;
} threadstore_t;

/******************************************************************************/

static void
__gthread_close_timer(threadentry_t *thr);

/******************************************************************************/

int
__gthread_active_p (void)
{
  /* Thread-system is always active as we have to be explicitly linked to the
   * final binary.
   */
  return 1;
}

/******************************************************************************/

int
__gthread_once (__gthread_once_t *__once, void (*__func) (void))
{
  __internal_gthread_once_t *once = (__internal_gthread_once_t *)__once;

  if (__once == NULL || __func == NULL)
    return EINVAL;

  /* Return only once __func has completed, also when another thread runs
   * it.  If __func throws, __gthread_once_unlock clears the started flag
   * and a waiting thread takes over.
   */
  while (!__atomic_load_1(&once->u.i.done, __ATOMIC_SEQ_CST))
    {
      if (!__atomic_test_and_set(&once->u.i.started, __ATOMIC_SEQ_CST))
        {
          /* Started flag was not set so call func now */
          __func();

          /* Remember that we are done now. And make all effects prior to this
           * store visible to all the other clients that will read that we are
           * actually done.
           */
          __atomic_store_1(&once->u.i.done, 1, __ATOMIC_SEQ_CST);
          break;
        }

      /* Allow the other thread to progress quickly but iexec may be not available */
      struct ExecBase *SysBase = *(struct ExecBase **)4;
      struct ExecIFace *ie = (struct ExecIFace *)SysBase->MainInterface;
      ie->Reschedule();
    }

  return 0;
}

void __gthread_once_unlock(__gthread_once_t *__once)
{
  __internal_gthread_once_t *once = (__internal_gthread_once_t *)__once;
  __atomic_store_1(&once->u.i.started, 0, __ATOMIC_SEQ_CST);
}

/******************************************************************************/

/* We keep the entries of each key organized as a single linked list */
struct keyentry
{
  struct keyentry *next;
  struct Task *task;
  const void *data;
};

typedef struct keyentry keyentry_t;

struct key
{
  keyentry_t *first;
  void (*destroy) (void *);
};

typedef struct
{
  struct SignalSemaphore sem;

  /* Each key is a single slot */
  struct key *keys;

  /* How many new keys */
  int num_key_entries;

  /* How many keys are allocated in total */
  int num_key_entries_allocated;

  /* Pool from which all key entries are allocated */
  APTR keyentry_pool;

  int num_threads;
} keystore_t;

static keystore_t *keystore;
static __gthread_once_t keystore_once = __GTHREAD_ONCE_INIT;

/**
 * Initialize our keystore.
 */
static void init_keystore(void)
{
  keystore_t *ks;

  if (!(ks = (keystore_t *)iexec->AllocVec (sizeof(*ks), MEMF_CLEAR)))
    return;
  iexec->InitSemaphore(&ks->sem);

  if (!(ks->keyentry_pool = iexec->AllocSysObjectTags (ASOT_ITEMPOOL, ASOITEM_ItemSize, sizeof(keyentry_t), TAG_DONE)))
    {
      iexec->FreeVec (ks);
      return;
    }

  /* Publish it only when it is complete: __gthread_entry reads keystore
   * without going through keystore_once.
   */
  __atomic_store_n(&keystore, ks, __ATOMIC_RELEASE);
}

int __gthread_key_create (__gthread_key_t *__key, void (*destroy) (void *))
{
  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  /* Initialize our private keystore, but only once */
  __gthread_once (&keystore_once, init_keystore);

  if (!keystore)
    return ENOMEM;

  iexec->ObtainSemaphore (&keystore->sem);
  if (keystore->num_key_entries >= keystore->num_key_entries_allocated)
    {
      struct key *new_keys;
      int new_num_key_entries_allocated = keystore->num_key_entries_allocated * 2 + 4;

      if (!(new_keys = (struct key *)iexec->AllocVec (sizeof(*new_keys) * new_num_key_entries_allocated, 0)))
        {
          iexec->ReleaseSemaphore (&keystore->sem);
          return ENOMEM;
        }
      if (keystore->keys)
        {
          memcpy (new_keys, keystore->keys, keystore->num_key_entries * sizeof(*new_keys));
          iexec->FreeVec (keystore->keys);
        }
      keystore->keys = new_keys;
      keystore->num_key_entries_allocated = new_num_key_entries_allocated;
    }
  keystore->keys[keystore->num_key_entries].first = NULL;
  keystore->keys[keystore->num_key_entries].destroy = destroy;

  __key->id = keystore->num_key_entries++;

  iexec->ReleaseSemaphore (&keystore->sem);
  return 0;
}

int
__gthread_key_delete (__gthread_key_t __key)
{
  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  iexec->ObtainSemaphore (&keystore->sem);

  /* TODO: Free the occupied storage for each entry */
  keystore->keys[__key.id].first = NULL;

  iexec->ReleaseSemaphore (&keystore->sem);
  return 0;
}

/**
 * Find the key entry that corresponds to id and task.
 *
 * @return the key entry or NULL.
 */
static keyentry_t *find_keyentry (int id, struct Task *task)
{
  keyentry_t *entry;
  entry = keystore->keys[id].first;
  while (entry)
    {
      if (entry->task == task)
          break;
      entry = entry->next;
    }
  return entry;
}

void *
__gthread_getspecific (__gthread_key_t __key)
{
  struct Task *this_task;
  keyentry_t *entry;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  this_task = iexec->FindTask(NULL);

  iexec->ObtainSemaphoreShared (&keystore->sem);
  entry = find_keyentry(__key.id, this_task);
  iexec->ReleaseSemaphore (&keystore->sem);
  return entry?((void*)entry->data):NULL;
}

int
__gthread_setspecific (__gthread_key_t __key, const void *__v)
{
  struct Task *this_task;
  keyentry_t *entry;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  this_task = iexec->FindTask(NULL);

  iexec->ObtainSemaphore (&keystore->sem);
  if (!(entry = find_keyentry(__key.id, this_task)))
    {
      /* Specific not found for this task, allocate a new entry */
      if (!(entry = (keyentry_t *)iexec->ItemPoolAlloc(keystore->keyentry_pool)))
        {
          iexec->ReleaseSemaphore (&keystore->sem);
          return ENOMEM;
        }
      entry->task = this_task;
      /* entry->data will be set in the following */
      entry->next = keystore->keys[__key.id].first;
      keystore->keys[__key.id].first = entry;
    }
  entry->data = __v;
  iexec->ReleaseSemaphore (&keystore->sem);
  return 0;
}

/******************************************************************************/

int
__gthread_mutex_destroy (__gthread_mutex_t *__mutex UNUSED)
{
  return 0;
}

int
__gthread_mutex_init (__gthread_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  iexec->InitSemaphore (&mx->u.i.sem);
  mx->u.i.recursive = 0;
  mx->u.i.acquired = 0;
  return 0;
}

int
__gthread_mutex_lock (__gthread_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  iexec->ObtainSemaphore (&mx->u.i.sem);
  if (!mx->u.i.recursive && mx->u.i.acquired)
    {
      /* Deadlock */
      iexec->DebugPrintF("threadimpl: non-recursive mutex_lock() called twice on the same thread.\n");
      iexec->ReleaseSemaphore (&mx->u.i.sem);
      return EBUSY;
    }
  mx->u.i.acquired++;
  return 0;
}

int
__gthread_mutex_trylock (__gthread_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  if (iexec->AttemptSemaphore (&mx->u.i.sem))
    {
      if (!mx->u.i.recursive && mx->u.i.acquired)
        {
          iexec->ReleaseSemaphore (&mx->u.i.sem);
          return EBUSY;
        }
      mx->u.i.acquired++;
      return 0;
    }
  return EBUSY;
}

int
__gthread_mutex_unlock (__gthread_mutex_t *__mutex)
{
  __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  mx->u.i.acquired--;
  iexec->ReleaseSemaphore (&mx->u.i.sem);
  return 0;
}

/******************************************************************************/

int
__gthread_recursive_mutex_init (__gthread_recursive_mutex_t *__mutex)
{
  int err;
  if (!(err = __gthread_mutex_init ((__gthread_mutex_t*)__mutex)))
    {
      __internal_gthread_mutex_t *mx = (__internal_gthread_mutex_t *)__mutex;
      mx->u.i.recursive = 1;
    }
  return err;
}

int
__gthread_recursive_mutex_lock (__gthread_recursive_mutex_t *__mutex)
{
  return __gthread_mutex_lock ((__gthread_mutex_t*)__mutex);
}

int
__gthread_recursive_mutex_trylock (__gthread_recursive_mutex_t *__mutex)
{
  return __gthread_mutex_trylock ((__gthread_mutex_t*)__mutex);
}

int
__gthread_recursive_mutex_unlock (__gthread_recursive_mutex_t *__mutex)
{
  return __gthread_mutex_unlock ((__gthread_mutex_t*)__mutex);
}

int
__gthread_recursive_mutex_destroy (__gthread_recursive_mutex_t *__mutex)
{
  return __gthread_mutex_destroy ((__gthread_mutex_t*)__mutex);
}

/* exec semaphores have no timed obtain, so poll until the deadline,
   sleeping a tick (1/50 s) between attempts.  */
int
__gthread_mutex_timedlock (__gthread_mutex_t *__mutex,
			   const __gthread_time_t *abs_timeout)
{
  struct timeval now;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  for (;;)
    {
      if (!__gthread_mutex_trylock (__mutex))
	return 0;
      gettimeofday (&now, 0);
      if (now.tv_sec > abs_timeout->seconds
	  || (now.tv_sec == abs_timeout->seconds
	      && now.tv_usec * 1000L >= abs_timeout->nanoseconds))
	return ETIMEDOUT;
      idos->Delay (1);
    }
}

int
__gthread_recursive_mutex_timedlock (__gthread_recursive_mutex_t *__mutex,
				     const __gthread_time_t *abs_timeout)
{
  return __gthread_mutex_timedlock ((__gthread_mutex_t *)__mutex, abs_timeout);
}

/******************************************************************************/

static threadstore_t *threadstore;
static __gthread_once_t threadstore_once = __GTHREAD_ONCE_INIT;

/**
 * Allocate data structure for a new thread and prepare it with
 * a new id.
 */
static threadentry_t *__gthread_new_threadentry (void)
{
  threadentry_t *thr;
  int thr_id;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  if (!(thr = (threadentry_t *)iexec->AllocVec (sizeof (*thr), MEMF_CLEAR)))
    return NULL;

  iexec->ObtainSemaphore (&threadstore->sem);
  thr_id = threadstore->next_thread_id++;
  iexec->ReleaseSemaphore (&threadstore->sem);

  thr->id = thr_id;

  return thr;
}

/**
 * Initialize our threadstore.
 */
static void init_threadstore (void)
{
  threadentry_t *thr;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  if (!(threadstore = (threadstore_t *)iexec->AllocVec (sizeof(*threadstore), MEMF_CLEAR)))
    return;
  iexec->InitSemaphore (&threadstore->sem);

  /* Create thread structure for this thread (aka root thread) */
  if (!(thr = __gthread_new_threadentry ()))
    goto bailout;
  thr->process = (struct Process*)iexec->FindTask(NULL);

  threadstore->threads = thr;
  return;
bailout:
  if (threadstore)
    iexec->FreeVec (threadstore);
  threadstore = NULL;
}

static int __gthread_entry(STRPTR args UNUSED, int32 length UNUSED, APTR execbase UNUSED)
{
  struct Task *task;
  threadentry_t *thr;
  keystore_t *ks;
  int i;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  task = iexec->FindTask (NULL);
  thr = (threadentry_t *)((struct Process*)task)->pr_Task.tc_UserData;

  /* Wait for the parent task to enqueue the process in the global list */
  while (!(iexec->Wait (SIGBREAKF_CTRL_F) & SIGBREAKF_CTRL_F));

  thr->result = thr->entry(thr->args);

  /* Invoke destructors of all non-NULL thread specifics.  There is no
   * keystore if no key was ever created.  Read it once: another thread
   * can create the first key meanwhile.
   */
  ks = __atomic_load_n(&keystore, __ATOMIC_ACQUIRE);
  if (ks)
    {
      iexec->ObtainSemaphore (&ks->sem);
      for (i = 0; i < ks->num_key_entries; i++)
        {
          void (*destroy)(void *);
          keyentry_t *key;

          if (!(destroy = ks->keys[i].destroy))
            continue;

          if (!(key = find_keyentry (i, task)))
            continue;

          if (key->data)
            destroy ((void *)key->data);
        }
      iexec->ReleaseSemaphore (&ks->sem);
    }

  __gthread_close_timer (thr);

  thr->finished = 1;
  return 0;
}

int
__gthread_create (__gthread_t *thread, void *(*func) (void*), void *args)
{
  threadentry_t *threadentry;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  /* Initialize our private threadstore, but only once */
  __gthread_once (&threadstore_once, init_threadstore);

  if (!threadstore)
    return ENOMEM;

  if (!(threadentry = __gthread_new_threadentry ()))
    return ENOMEM;

  threadentry->entry = func;
  threadentry->args = args;

  threadentry->process = idos->CreateNewProcTags (
                NP_Entry, __gthread_entry,
                NP_Child, TRUE,
                NP_UserData, (int32)threadentry,
                NP_Input, idos->Input(),
                NP_Output, idos->Output(),
                NP_Error, idos->ErrorOutput(),
                NP_CloseInput,  FALSE,
                NP_CloseOutput, FALSE,
                NP_CloseError, FALSE,
                TAG_DONE);

  iexec->ObtainSemaphore (&threadstore->sem);
  threadentry->next = threadstore->threads;
  threadstore->threads = threadentry;
  iexec->ReleaseSemaphore (&threadstore->sem);

  /* Signal that we have enqueued the task */
  iexec->Signal (&threadentry->process->pr_Task, SIGBREAKF_CTRL_F);

  *thread = threadentry->id;
  return 0;
}

static threadentry_t *find_threadentry_by_process (struct Process *process)
{
  threadentry_t *thr;
  thr = threadstore->threads;
  while (thr)
    {
      if (thr->process == process)
          break;
      thr = thr->next;
    }
  return thr;
}

static threadentry_t *find_threadentry_by_id (__gthread_t id)
{
  threadentry_t *thr;
  thr = threadstore->threads;
  while (thr)
    {
      if (thr->id == id)
          break;
      thr = thr->next;
    }
  return thr;
}

int
__gthread_join (__gthread_t thread, void **value_ptr)
{
  threadentry_t *thr;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  if (!(thr = find_threadentry_by_id (thread)))
    return ESRCH;

  /* FIXME: This is very ugly, use SIGF_SINGLE */
  while (!thr->finished)
    idos->Delay (1);

  if (value_ptr)
    *value_ptr = thr->result;
  return 0;
}

int
__gthread_detach (__gthread_t thread)
{
  threadentry_t *thr = find_threadentry_by_id (thread);
  thr->detached = 1;
  return 0;
}

int
__gthread_equal (__gthread_t t1, __gthread_t t2)
{
  return t1 == t2;
}

__gthread_t
__gthread_self (void)
{
  struct Task *task;
  threadentry_t *thr;
  __gthread_t id;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  /* Initialize our private threadstore, but only once */
  __gthread_once (&threadstore_once, init_threadstore);

  task = iexec->FindTask(NULL);
  iexec->ObtainSemaphoreShared (&threadstore->sem);
  if ((thr = find_threadentry_by_process ((struct Process *)task)))
    {
      id = thr->id;
    }
  else
    /* FIXME: Record this thread */
    id = -1;

  iexec->ReleaseSemaphore (&threadstore->sem);
  return id;
}

int
__gthread_yield (void)
{
  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  iexec->Reschedule();
  return 0;
}

void
__gthread_close_timer(threadentry_t *thr)
{
  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  if (thr->timer_io)
    {
      if (thr->timer_io->Request.io_Device)
        {
          iexec->CloseDevice (&thr->timer_io->Request);
        }
      iexec->FreeSysObject (ASOT_IOREQUEST, thr->timer_io);
      thr->timer_io = NULL;
    }

  if (thr->timer_port)
    {
      iexec->DeleteMsgPort (thr->timer_port);
      thr->timer_port = NULL;
    }
}

static int
__gthread_open_timer(threadentry_t **thr_p)
{
  threadentry_t *thr;
  struct Task *task;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  /* Initialize our private threadstore, but only once.  This may be the
   * first gthread call of the process.
   */
  __gthread_once (&threadstore_once, init_threadstore);

  if (!threadstore)
    return ENOMEM;

  task = iexec->FindTask(NULL);

  iexec->ObtainSemaphoreShared (&threadstore->sem);
  if (!(thr = find_threadentry_by_process((struct Process*)task)))
    {
      /* This is not our thread */
      iexec->ReleaseSemaphore (&threadstore->sem);
      return EPERM;
    }
  iexec->ReleaseSemaphore (&threadstore->sem);

  /* Only we can open the timer device */
  if (!thr->timer_port)
    {
      if (!(thr->timer_port = iexec->CreateMsgPort ()))
        goto bailout;

      if (!(thr->timer_io = (struct TimeRequest *)iexec->AllocSysObjectTags (ASOT_IOREQUEST,
          ASOIOR_Size, sizeof(struct TimeRequest),
          ASOIOR_ReplyPort, thr->timer_port,
          TAG_END)))
        goto bailout;

      if (iexec->OpenDevice(TIMERNAME, UNIT_MICROHZ, &thr->timer_io->Request, 0))
        goto bailout;
    }

  *thr_p = thr;
  return 0;
bailout:
  __gthread_close_timer(thr);
  return ENOMEM;
}

/**
 * Close the timer of the main task at exit.  __gthread_entry closes the
 * timers of the threads it runs, but the main task does not go through it.
 */
static void __attribute__((destructor))
__gthread_close_main_timer (void)
{
  threadentry_t *thr;

  if (!threadstore)
    return;

  iexec->ObtainSemaphoreShared (&threadstore->sem);
  thr = find_threadentry_by_process ((struct Process *)iexec->FindTask (NULL));
  iexec->ReleaseSemaphore (&threadstore->sem);

  if (thr)
    __gthread_close_timer (thr);
}

/******************************************************************************/

/**
 * Remove the given thread from the condition wait list.
 */
static void
__gthread_remove_thread_from_cond_wait_list (__internal_gthread_cond_t *cond, threadentry_t *thr)
{
  threadentry_t *cur_thr, *prev_thr;

  prev_thr = NULL;
  cur_thr = cond->u.i.first_in_cond_wait_list;
  while (cur_thr)
    {
      if (cur_thr == thr)
        {
          if (!prev_thr)
            cond->u.i.first_in_cond_wait_list = cur_thr->next_in_cond_wait_list;
          else
            prev_thr->next_in_cond_wait_list = cur_thr->next_in_cond_wait_list;
          break;
        }

      prev_thr = cur_thr;
      cur_thr = cur_thr->next_in_cond_wait_list;
    }
}

int
__gthread_cond_init (__gthread_cond_t *cond)
{
  memset(cond, 0, sizeof(*cond));
  return 0;
}

int
__gthread_cond_signal (__gthread_cond_t *__cond)
{
  __internal_gthread_cond_t *cond = (__internal_gthread_cond_t *)__cond;
  threadentry_t *thr;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  /* Initialize our private threadstore, but only once */
  __gthread_once (&threadstore_once, init_threadstore);

  if (!threadstore)
    return ENOMEM;

  /* Most of the time, we are called with the same mutex being locked that is
   * supplied to __gthread_cond_wait(). However,  most of the time is not always,
   * so we have to make the lists manipulation atomic on our own.
   */
  iexec->ObtainSemaphore (&threadstore->sem);

  if ((thr = cond->u.i.first_in_cond_wait_list))
    {
      /* Remove us from the list, then signal */
      cond->u.i.first_in_cond_wait_list = thr->next_in_cond_wait_list;
      thr->next_in_cond_wait_list = NULL;

      iexec->Signal (&thr->process->pr_Task, SIGF_SINGLE);
    }

  iexec->ReleaseSemaphore (&threadstore->sem);

  return 0;
}

int
__gthread_cond_broadcast (__gthread_cond_t *__cond)
{
  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  /* Initialize our private threadstore, but only once */
  __gthread_once (&threadstore_once, init_threadstore);

  __internal_gthread_cond_t *cond = (__internal_gthread_cond_t *)__cond;
  threadentry_t *thr;

  /* See __gthread_cond_signal() */
  iexec->ObtainSemaphore (&threadstore->sem);

  thr = cond->u.i.first_in_cond_wait_list;
  cond->u.i.first_in_cond_wait_list = NULL;

  while (thr)
    {
      threadentry_t *next_thr = thr->next_in_cond_wait_list;

      thr->next_in_cond_wait_list = NULL;

      iexec->Signal (&thr->process->pr_Task, SIGF_SINGLE);

      thr = next_thr;
    }

  iexec->ReleaseSemaphore (&threadstore->sem);

  return 0;
}

/**
 * Prepare cond_wait.
 */
static void
__gthread_cond_wait_prepare (threadentry_t *self_thr, __internal_gthread_cond_t *cond, __gthread_mutex_t *mutex)
{
  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  /* Enqueue us into this thread's dedicated cond wait list */
  iexec->ObtainSemaphore (&threadstore->sem);
  self_thr->next_in_cond_wait_list = cond->u.i.first_in_cond_wait_list;
  cond->u.i.first_in_cond_wait_list = self_thr;

  /* Along the way, clear SIGF_SINGLE bit to avoid spurious signals */
  iexec->SetSignal (0L, SIGF_SINGLE);

  iexec->ReleaseSemaphore (&threadstore->sem);

  /* TODO: Set a condition variable, which the notifier is supposed to change
   * to avoid more suspsious signals. However, the caller is supposed to check
   * the condition anyway, so suspsious wakes are perhaps acceptable.
   */
  __gthread_mutex_unlock (mutex);
}

int
__gthread_cond_wait (__gthread_cond_t *__cond, __gthread_mutex_t *mutex)
{
  __internal_gthread_cond_t *cond = (__internal_gthread_cond_t *)__cond;
  __gthread_t self = __gthread_self ();
  threadentry_t *self_thr;

  if (!(self_thr = find_threadentry_by_id (self)))
    return EINVAL;

  __gthread_cond_wait_prepare (self_thr, cond, mutex);

  /* Wait for anyone notifiying us */
  while (!(iexec->Wait (SIGF_SINGLE) & SIGF_SINGLE));

  __gthread_mutex_lock (mutex);

  /* Receiving the proper SIGF_SINGLE also means that we have been removed from
   * the cond's waiting list
   */
  return 0;
}

int
__gthread_cond_timedwait (__gthread_cond_t *__cond,
                          __gthread_mutex_t *mutex,
                          const __gthread_time_t *abs_timeout)
{
  __internal_gthread_cond_t *cond = (__internal_gthread_cond_t *)__cond;
  threadentry_t *self_thr;
  uint32_t timer_mask;
  struct timeval tv_now;
  uint32_t sigs;
  int borrow = 0;
  int err;

  /* Initialize libs */
  __gthread_once (&libs_once, init_libs);

  gettimeofday(&tv_now, 0);

  /* Some timer arithmetics. TODO: Verify this! */
  if (tv_now.tv_usec <= (uint32_t)(abs_timeout->nanoseconds / 1000))
    tv_now.tv_usec = abs_timeout->nanoseconds / 1000 - tv_now.tv_usec;
  else
    {
      tv_now.tv_usec = 1000000 + abs_timeout->nanoseconds / 1000 - tv_now.tv_usec;
      borrow = 1;
    }

  if (tv_now.tv_sec + borrow <= (uint32_t)(abs_timeout->seconds))
    tv_now.tv_sec = abs_timeout->seconds - (tv_now.tv_sec + borrow);
  else
    {
      tv_now.tv_sec = 0;
      tv_now.tv_usec = 1;
    }

  if ((err = __gthread_open_timer (&self_thr)))
    return err;

  self_thr->timer_io->Request.io_Command = TR_ADDREQUEST;
  self_thr->timer_io->Time.Seconds = tv_now.tv_sec;
  self_thr->timer_io->Time.Microseconds = tv_now.tv_usec;
  iexec->SendIO (&self_thr->timer_io->Request);

  __gthread_cond_wait_prepare (self_thr, cond, mutex);

  /* Wait for anyone notifiying us or a timeout, note that according to
   * some docs we should not wait on other signals when using SIGF_SINGLE,
   * but we do it nonetheless.  */
  timer_mask = 1UL << self_thr->timer_port->mp_SigBit;
  sigs = iexec->Wait (SIGF_SINGLE | timer_mask);

  if (!(iexec->CheckIO (&self_thr->timer_io->Request)))
    iexec->AbortIO (&self_thr->timer_io->Request);
  iexec->WaitIO (&self_thr->timer_io->Request);

  __gthread_mutex_lock (mutex);

  /* Receiving the proper SIGF_SINGLE also means that we have been removed from
   * the cond's waiting list, but if this was a timeout, we have not yet been
   * removed. So we remove ourselves. This is safe, even if we have been removed.
   */
  if (!(sigs & SIGF_SINGLE))
    {
      iexec->ObtainSemaphore (&threadstore->sem);
      __gthread_remove_thread_from_cond_wait_list (cond, self_thr);
      iexec->ReleaseSemaphore (&threadstore->sem);
    }
  return err;
}

int
__gthread_cond_wait_recursive (__gthread_cond_t *__cond,
			       __gthread_recursive_mutex_t *__mutex)
{
  /* Recursive and plain mutexes share the representation.  */
  return __gthread_cond_wait (__cond, (__gthread_mutex_t *)__mutex);
}

int
__gthread_cond_destroy (__gthread_cond_t *__cond UNUSED)
{
  return 0;
}

#ifdef __cplusplus
}
#endif
