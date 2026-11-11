/* { dg-do run } */
/* { dg-shouldfail "the libraries are older than __oslibversion" } */
/* oslibversion-run.c with newlib, which amigaos.exp puts in place of the
   board's runtime: its crt0 has a list of libraries of its own.  */
#include "oslibversion-run.c"

/* { dg-output "dos.library failed to open" } */
