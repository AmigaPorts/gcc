/* frame-addr-store.c with ptr addressed from a4.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68000 -fomit-frame-pointer -fbaserel" } */

#include "frame-addr-store.c"

/* { dg-final { scan-assembler-times {move\.l sp,_ptr:W\(a4\)} 2 } } */
/* { dg-final { scan-assembler-not "add\\.l #-4096,d" } } */
/* { dg-final { scan-assembler-not "add\\.l #-65536,d" } } */
