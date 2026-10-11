/* An asmreg argument narrower than int is passed in its declared mode: the
   caller loads the word into d1 without clearing the upper half first.  */
/* { dg-do compile } */
/* { dg-options "-O2 -fomit-frame-pointer" } */

typedef unsigned short UWORD;

extern void cw (register UWORD x __asm("d1"));

void
tw (int v)
{
  cw (v);
}

/* { dg-final { scan-assembler "move.w 6\\(sp\\),d1" } } */
/* { dg-final { scan-assembler-not "moveq #0,d1" } } */
