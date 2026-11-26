/* { dg-do compile } */
/* { dg-options "-Os -freorder-blocks-algorithm=simple -fdump-rtl-bbro" } */

#include "block-reorder-size.c"

/* { dg-final { scan-rtl-dump {Reordering with the "simple" algorithm} "bbro" } } */
