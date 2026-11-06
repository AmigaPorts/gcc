/* Do not reorder potentially throwing loads.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68020 -fnon-call-exceptions" } */

#include "load-compare.h"

/* { dg-final { scan-assembler-times {tst\.[bwl]} 8 } } */
