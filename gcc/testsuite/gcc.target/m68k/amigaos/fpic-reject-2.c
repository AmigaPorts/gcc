/* Same as fpic-reject.c for the -fPIC spelling (flag_pic 2).  Named -2
   rather than fPIC-reject.c: a name differing only in case collides with
   fpic-reject.c when the tree is checked out on a case-insensitive file
   system, as macOS has by default.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga pic modes" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-fPIC" } */
/* { dg-error "-fPIC.*not supported on this target" "" { target *-*-* } 0 } */

int dummy;
