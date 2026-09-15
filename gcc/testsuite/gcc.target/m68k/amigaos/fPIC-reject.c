/* Same as fpic-reject.c for the -fPIC spelling (flag_pic 2).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga pic modes" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-fPIC" } */
/* { dg-error "-fPIC.*not supported on this target" "" { target *-*-* } 0 } */

int dummy;
