/* { dg-do compile } */
/* { dg-options "-Os -m68010" } */

#include "mul-neg-power-000.c"

/* { dg-final { scan-assembler-not {__mulsi3|mul[su][.]?[wl]} } } */
