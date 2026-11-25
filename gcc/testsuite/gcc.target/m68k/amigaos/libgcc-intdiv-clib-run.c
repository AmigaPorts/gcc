/* libgcc's 68000 multiply and divide helpers are weak, so they never clash
   with a C library's.  Here the unsigned % links libnix's __udivsi3, then
   libnix's printf needs __divsi3 after ld has left libnix20.a, and takes
   libgcc's, which brings libgcc's __udivsi3 through __udivsi3_internal:
   "multiple definition of `__udivsi3'" if libgcc's were strong.  */

/* { dg-do run } */
/* { dg-skip-if "the 68000 has no -fbaserel32" { *-*-* } { "-fbaserel32" "-resident32" } { "" } } */
/* { dg-options "-m68000 -O2" } */

#include <stdio.h>
#include <string.h>

static volatile unsigned long a = 1234567, b = 1000;

int
main (void)
{
  char s[16];

  sprintf (s, "%lu", a % b);
  return strcmp (s, "567") != 0;
}
