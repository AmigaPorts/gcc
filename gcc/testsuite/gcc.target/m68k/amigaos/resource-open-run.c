/* { dg-do run } */
/* { dg-require-effective-target amigaos_battclock } */
/* { dg-skip-if "ixemul opens no libraries for the program" { *-*-* } { "-mcrt=ixemul" } { "" } } */
/* { dg-skip-if "clib2 opens no libraries for the program" { *-*-* } { "-mcrt=clib2" } { "" } } */

/* The startup code opens what a program uses before main(), from the list
   of base variables and names that libstubs provides.  A name that ends in
   ".resource" is opened with OpenResource() instead of OpenLibrary(), and
   is not closed at exit: a resource has no close.  */

#include <proto/battclock.h>

int
main (void)
{
  if (!BattClockBase)
    return 1;
  /* A call through the base; the time is the board's business.  */
  ReadBattClock ();
  return 0;
}
