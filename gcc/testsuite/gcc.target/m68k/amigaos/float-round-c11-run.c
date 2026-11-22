/* { dg-do run } */
/* { dg-options "-std=c11" } */
/* float-round-run.c with the standard excess precision that -std=c11
   implies.  With -flto, lto1 defaults to -fexcess-precision=fast, while
   each function keeps the precision it was compiled with.  */

#include "float-round-run.c"
