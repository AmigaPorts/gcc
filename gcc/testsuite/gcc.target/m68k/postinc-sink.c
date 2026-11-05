/* Late sinking must not split a post-increment load across blocks.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68020 -fno-code-hoisting" } */

#include "byte-compare.h"

/* { dg-final { scan-assembler-not {addq\.l} } } */
/* { dg-final { scan-assembler-times {move\.b \(%?a[0-6]\)\+,%?d[0-7]} 2 } } */
