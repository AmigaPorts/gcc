/* Test that function call arguments aren't clobbered by register assignment */
/* { dg-do compile } */
/* { dg-options "-O2 -noixemul" } */
/* { dg-final { scan-assembler "move.l d0,d2" } } */

#include <proto/graphics.h>

extern ULONG pen_rgb(ULONG color);

void test(struct RastPort *rp) {
    SetABPenDrMd(rp, pen_rgb(0x5A), pen_rgb(0xA5), 1);
}
