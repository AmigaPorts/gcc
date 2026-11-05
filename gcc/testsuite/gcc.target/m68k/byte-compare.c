/* Hoisting and sinking should retain a five-instruction byte loop.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68020" } */

#include "byte-compare.h"

/* { dg-final { scan-assembler-not {addq\.l} } } */
/* { dg-final { scan-assembler-not {tst\.b} } } */
/* { dg-final { scan-assembler-times {move\.b \(%?a[0-6]\)\+,%?d[0-7]} 2 } } */
