// -athread=pthread, which needs clib4.
// { dg-do run }
// { dg-require-effective-target amigaos_clib4 }
// { dg-options "-O2 -athread=pthread" }

#include "hello-run.C"
