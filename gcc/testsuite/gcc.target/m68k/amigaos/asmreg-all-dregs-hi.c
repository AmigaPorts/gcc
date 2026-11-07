/* Arguments in all eight data registers, passed on to a call in the same
   registers, keep every data register busy up to the call.  A short
   tested before the call used to get an address register, and LRA looped
   forever splitting hard regs for the data-register reload that the HImode
   compare then needed (AmigaPorts/m68k-amigaos-gcc#309, reduced from the
   ZZ9000 Picasso96 driver's BlitPlanar2Chunky).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68020" } */

typedef short SHORT;
typedef char UBYTE;

struct BoardInfo {
  void (*Default) (struct BoardInfo *, int *, int *,
		   SHORT __asm ("d0"), SHORT __asm ("d1"),
		   SHORT __asm ("d2"), SHORT __asm ("d3"),
		   SHORT __asm ("d4"), SHORT __asm ("d5"),
		   UBYTE __asm ("d6"), UBYTE __asm ("d7"));
};

struct BoardInfo b;
int bm, r;

void
f (SHORT x __asm ("d0"), SHORT y __asm ("d1"),
   SHORT dx __asm ("d2"), SHORT dy __asm ("d3"),
   SHORT w __asm ("d4"), SHORT h __asm ("d5"),
   UBYTE minterm __asm ("d6"), UBYTE mask __asm ("d7"))
{
  if (w || h < 1)
    return;
  b.Default (&b, &bm, &r, x, y, dx, dy, w, h, minterm, mask);
}
