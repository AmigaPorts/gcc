/* --gc-sections with ixemul's crt0.o, an a.out object, first in the link:
   crt0.o refers to _main but does not define it, so main's file must still
   be followed and keep gc_helper, which only main calls.  amigaos.exp runs
   this file with -mcrt=ixemul.  */

/* { dg-do link } */
/* { dg-options "-Os -Wl,--gc-sections -Wl,-Map=gc-sections-ixemul-link.map" } */
/* { dg-additional-sources "gc-sections-ixemul-link-lib.c" } */

#include <stdio.h>

int gc_helper (int);

int
main (int argc, char **argv)
{
  printf ("%d\n", gc_helper (argc));
  return 0;
}

/* { dg-final { scan-file gc-sections-ixemul-link.map {0x[0-9a-f]+ +gc_helper\n} } } */
/* { dg-final { remove-build-file "gc-sections-ixemul-link.map" } } */
