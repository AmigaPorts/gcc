/* Reorder independent integer loads to reuse the tested load's flags.
   Keep volatile loads and indexed-address dependencies in order.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68020" } */

#include "load-compare.h"

/* Three volatile cases and one address dependency still need a test.  */
/* { dg-final { scan-assembler-times {tst\.[bwl]} 4 } } */
