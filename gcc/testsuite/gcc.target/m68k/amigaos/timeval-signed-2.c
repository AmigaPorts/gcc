/* POSIX's tv_sec and tv_usec are signed, whichever header defines struct
   timeval: here NDK 3.2's <devices/timer.h> comes before the C library's
   <sys/time.h>, and the Amiga names are there too.  */
/* { dg-do compile } */

#include <devices/timer.h>
#include <sys/time.h>

#define SIGNED(m) ((__typeof__ (((struct timeval *) 0)->m)) -1 < 0)

_Static_assert (SIGNED (tv_sec), "tv_sec is signed");
_Static_assert (SIGNED (tv_usec), "tv_usec is signed");
_Static_assert (!SIGNED (tv_secs), "tv_secs is unsigned");
_Static_assert (!SIGNED (tv_micro), "tv_micro is unsigned");
_Static_assert (sizeof (struct timeval) == 8, "two 32-bit fields");
