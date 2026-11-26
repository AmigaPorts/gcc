/* { dg-do compile } */
/* { dg-options "-Oz -fdump-rtl-bbro" } */

#include "block-reorder-size.c"

/* { dg-final { scan-rtl-dump {Reordering with the STC algorithm} "bbro" } } */
