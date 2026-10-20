/* An ixemul program built for the 68020 must link ixemul's crt0.o.  The
   startfile spec named it crt0.o%s, and that search tries the multilib
   subdirectory of every prefix first, where newlib installs a crt0.o of the
   same name: -m68020 linked newlib's startup file against ixemul's libc.a
   and failed on __call_exitprocs and DOSBase.  amigaos.exp runs this file
   with -mcrt=ixemul in place of the board's runtime.  */

/* { dg-do link } */
/* { dg-skip-if "amiga ixemul" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68020" } */

#include <stdio.h>

int
main (int argc, char **argv)
{
  printf ("%d %s\n", argc, argv[0]);
  return 0;
}
