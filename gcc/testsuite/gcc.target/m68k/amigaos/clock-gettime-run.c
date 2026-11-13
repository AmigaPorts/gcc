/* { dg-do run } */
/* { dg-skip-if "clib2 has no clock_gettime()" { *-*-* } { "-mcrt=clib2" } { "" } } */
/* clock_gettime (CLOCK_REALTIME) reads the clock of gettimeofday(),
   CLOCK_MONOTONIC advances by the time the program sleeps, and
   clock_getres() says a resolution below a second for both.  */
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <proto/dos.h>

static long long
micro (const struct timeval *tv)
{
  return tv->tv_sec * 1000000LL + tv->tv_usec;
}

static long long
nano (const struct timespec *ts)
{
  return ts->tv_sec * 1000000000LL + ts->tv_nsec;
}

static void
check_res (clockid_t id)
{
  struct timespec res;

  if (clock_getres (id, &res))
    abort ();
  if (res.tv_sec != 0 || res.tv_nsec <= 0 || res.tv_nsec >= 1000000000)
    abort ();
}

int
main (void)
{
  struct timeval before, after;
  struct timespec ts, later;
  long long slept;

  gettimeofday (&before, 0);
  if (clock_gettime (CLOCK_REALTIME, &ts))
    abort ();
  gettimeofday (&after, 0);
  if (ts.tv_nsec < 0 || ts.tv_nsec >= 1000000000)
    abort ();
  if (nano (&ts) / 1000 < micro (&before) || nano (&ts) / 1000 > micro (&after))
    abort ();
  check_res (CLOCK_REALTIME);

  if (clock_gettime (CLOCK_MONOTONIC, &ts))
    abort ();
  Delay (TICKS_PER_SECOND / 5);
  if (clock_gettime (CLOCK_MONOTONIC, &later))
    abort ();
  if (later.tv_nsec < 0 || later.tv_nsec >= 1000000000)
    abort ();
  /* a fifth of a second, give or take a tick, and a slow machine */
  slept = nano (&later) - nano (&ts);
  if (slept < 150000000 || slept > 5000000000LL)
    abort ();
  check_res (CLOCK_MONOTONIC);
  return 0;
}
