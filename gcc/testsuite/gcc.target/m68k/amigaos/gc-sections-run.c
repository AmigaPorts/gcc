/* --gc-sections must keep the constructor of gc-sections-run-lib.c, an
   unreferenced file: its .list_ entry makes the startup code call it, as
   with a module that registers itself.  */
/* { dg-do run } */
/* { dg-options "-Wl,--gc-sections" } */
/* { dg-additional-sources "gc-sections-run-lib.c" } */

#include <stdio.h>

int flag;

int
main (void)
{
  printf ("flag %d\n", flag);
  return flag == 42 ? 0 : 1;
}

/* { dg-output "flag 42" } */
