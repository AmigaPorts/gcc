/* The 68000 has no 32-bit multiply: at -Os the address computation of
   this loop, iv * -16, is a shift and a neg, not a call to __mulsi3.
   AmigaPorts/m68k-amigaos-gcc#421.  */
/* { dg-do compile } */
/* { dg-options "-m68000 -Os -ffreestanding" } */

typedef unsigned long ULONG;
typedef unsigned short UWORD;
#define AUD(ch, off) (0xdff0a0 + (ch) * 16 + (off))

void
setaud (ULONG ptr, ULONG n, UWORD m)
{
  int ch;
  for (ch = 0; ch < 4; ch++, m >>= 1)
    if (m & 1)
      {
	*(volatile ULONG *) AUD (ch, 0) = ptr;
	*(volatile UWORD *) AUD (ch, 4) = (UWORD) n >> 1;
      }
}

/* { dg-final { scan-assembler-not "__mulsi3" } } */
