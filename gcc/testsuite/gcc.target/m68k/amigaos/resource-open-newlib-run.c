/* { dg-do run } */
/* { dg-require-effective-target amigaos_battclock } */
/* resource-open-run.c with newlib, which amigaos.exp puts in place of the
   board's runtime: its crt0 walks the list of bases itself.  */
#include "resource-open-run.c"
