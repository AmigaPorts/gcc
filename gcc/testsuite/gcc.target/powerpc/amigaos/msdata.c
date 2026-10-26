/* Small data, which clib4 has no libraries for.  */
/* { dg-do run } */
/* { dg-skip-if "clib4 has no small-data libraries" { amigaos_clib4 } } */
/* { dg-options "-O2 -msdata=sysv" } */

#include "hello-run.c"
