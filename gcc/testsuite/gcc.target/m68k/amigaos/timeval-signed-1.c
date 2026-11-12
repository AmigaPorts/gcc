/* POSIX's tv_sec and tv_usec are signed, whichever header defines struct
   timeval: here the C library's <sys/time.h> comes before NDK 3.2's
   <devices/timer.h>.  timeval-signed-2.c tries the other order.  */
/* { dg-do compile } */
/* { dg-xfail-if "clib2 defines its own struct timeval, with unsigned fields" { *-*-* } { "-mcrt=clib2" } } */

#include <sys/time.h>
#include <devices/timer.h>

#define SIGNED(m) ((__typeof__ (((struct timeval *) 0)->m)) -1 < 0)

_Static_assert (SIGNED (tv_sec), "tv_sec is signed");
_Static_assert (SIGNED (tv_usec), "tv_usec is signed");
_Static_assert (sizeof (struct timeval) == 8, "two 32-bit fields");
