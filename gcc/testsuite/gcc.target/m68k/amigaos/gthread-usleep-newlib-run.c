/* { dg-do run } */
/* { dg-require-effective-target amigaos_gthreads } */
/* gthread-usleep-run.c with newlib, whose usleep() is dos.library's
   Delay(), where libnix clones dos.library's timer request.  */
#include "gthread-usleep-run.c"
