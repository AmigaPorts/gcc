/* { dg-do run } */
/* The C library's clock next to dos.library's DateStamp(), to a tick:
   - gettimeofday() is whole minutes away from the DateStamp, and time()
     agrees with it;
   - localtime() gives the DateStamp's own date and time back, except with
     ixemul, and mktime() undoes localtime();
   - neither setlocale() nor a TZ variable moves the clock.
   How far Unix time is from the DateStamp is the library's choice: clib2,
   ixemul and newlib add the UTC offset of the default locale to the epoch,
   libnix takes the clock as UTC.  */
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <dos/dos.h>
#include <proto/dos.h>

#define TICK_USEC (1000000 / TICKS_PER_SECOND)

/* Whether local time is the system clock's.  Not with ixemul: without TZ
   its local time is UTC, as on a Unix system with no zone set, whatever
   zone the system clock is in.  */
#ifdef __ixemul__
#define LOCAL_IS_CLOCK 0
#else
#define LOCAL_IS_CLOCK 1
#endif

/* Ticks since 1978.  */
typedef long long ticks_t;

static ticks_t
stamp (void)
{
  struct DateStamp ds;

  DateStamp (&ds);
  return (ds.ds_Days * 1440LL + ds.ds_Minute) * 60 * TICKS_PER_SECOND + ds.ds_Tick;
}

/* gettimeofday() between two DateStamps a tick apart at most, and the first
   of the two in *BEFORE.  */
static void
now (struct timeval *tv, ticks_t *before)
{
  time_t t0, t1;
  int i;

  for (i = 0; i < 20; i++)
    {
      *before = stamp ();
      t0 = time (0);
      gettimeofday (tv, 0);
      t1 = time (0);
      if (stamp () - *before <= 1)
	break;
    }
  if (i == 20)
    abort ();
  if (tv->tv_usec < 0 || tv->tv_usec >= 1000000)
    abort ();
  if (tv->tv_sec < t0 || tv->tv_sec > t1)
    abort ();
}

/* What gettimeofday() is ahead of the DateStamp by, in ticks.  */
static ticks_t
clock_offset (void)
{
  struct timeval tv;
  ticks_t before;

  now (&tv, &before);
  return (ticks_t) tv.tv_sec * TICKS_PER_SECOND + tv.tv_usec / TICK_USEC - before;
}

static void
check_offset (ticks_t offset)
{
  ticks_t d = clock_offset () - offset;

  if (d < -2 || d > 2)
    abort ();
}

static void
check_local (void)
{
  struct timeval tv;
  ticks_t before, local;
  struct tm tm;
  time_t t;
  long days;

  now (&tv, &before);
  t = tv.tv_sec;
  tm = *localtime (&t);
  /* every fourth year is a leap year until 2100 */
  days = (tm.tm_year - 78) * 365 + (tm.tm_year - 77) / 4 + tm.tm_yday;
  local = (((days * 24LL + tm.tm_hour) * 60 + tm.tm_min) * 60 + tm.tm_sec)
	  * TICKS_PER_SECOND + tv.tv_usec / TICK_USEC;
  if (LOCAL_IS_CLOCK && (local < before - 1 || local > before + 2))
    abort ();
  tm.tm_isdst = -1;
  if (mktime (&tm) != t)
    abort ();
}

int
main (void)
{
  ticks_t offset = clock_offset (), odd;

  /* an epoch and a time zone, whole minutes both */
  odd = offset % (60 * TICKS_PER_SECOND);
  if (odd > 2 && odd < 60 * TICKS_PER_SECOND - 2)
    abort ();

  check_local ();
  setlocale (LC_ALL, "C");
  check_offset (offset);
  check_local ();
  setlocale (LC_ALL, "");
  check_offset (offset);
  check_local ();

#if !defined __KICK13__ && !defined __CLIB2__
  /* localtime() may follow TZ, the clock may not.  Not with nix13, which
     has no setenv(), nor with clib2, whose tzset() does nothing.  */
  {
    char *old = getenv ("TZ");

    if (old)
      old = strdup (old);
    setenv ("TZ", "EST5", 1);
    tzset ();
    check_offset (offset);
    if (old)
      setenv ("TZ", old, 1);
    else
      unsetenv ("TZ");
  }
#endif
  return 0;
}
