/* A byte loaded with moveq #0 and move.b has a zero upper word, so shifting
   it left by 16, or by 24 after an lsl.w #8, needs only a swap: no clr.w.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68000" } */

unsigned int
load_be32 (const unsigned char *p)
{
  return ((unsigned int) p[0] << 24) | ((unsigned int) p[1] << 16)
	 | (p[2] << 8) | p[3];
}

/* { dg-final { scan-assembler-times "swap" 2 } } */
/* { dg-final { scan-assembler-not "clr\\.w" } } */
