/* Arguments in all eight data registers, passed on to a call in the same
   registers, keep every data register busy up to the call.  A byte read
   before it, bm->Depth, is tested with 0 and used again later.  IRA left
   it in memory, but LRA then gave it an address register, which no byte
   compare takes, and found no data register to reload it into: "unable
   to find a register to spill", or an endless loop before the LRA split
   limit.  A byte pseudo must stay out of the address registers.  Reduced
   from the ZZ9000 Picasso96 driver's BlitPlanar2Chunky.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68020 -fomit-frame-pointer" } */

typedef unsigned char UBYTE;
typedef short SHORT;
typedef unsigned long ULONG;

struct BitMap {
  unsigned short BytesPerRow, Rows;
  UBYTE Flags, Depth;
  unsigned short pad;
  UBYTE *Planes[8];
};

struct BoardInfo {
  void *MemoryBase;
  ULONG CardFlags;
  void (*Default) (struct BoardInfo *b __asm ("a0"),
		   struct BitMap *bm __asm ("a1"),
		   SHORT x __asm ("d0"), SHORT y __asm ("d1"),
		   SHORT dx __asm ("d2"), SHORT dy __asm ("d3"),
		   SHORT w __asm ("d4"), SHORT h __asm ("d5"),
		   UBYTE minterm __asm ("d6"), UBYTE mask __asm ("d7"));
};

extern volatile unsigned short regs[16];

void
f (struct BoardInfo *b __asm ("a0"), struct BitMap *bm __asm ("a1"),
   SHORT x __asm ("d0"), SHORT y __asm ("d1"), SHORT dx __asm ("d2"),
   SHORT dy __asm ("d3"), SHORT w __asm ("d4"), SHORT h __asm ("d5"),
   UBYTE minterm __asm ("d6"), UBYTE mask __asm ("d7"))
{
  ULONG addr = 0x100000;
  unsigned short line_size = ((w + 15) >> 4) << 1;
  ULONG plane_size = line_size * h;

  if (!(b->CardFlags & 4)
      && !(bm->Depth == 0 || plane_size <= 0x10000UL / bm->Depth))
    {
      b->Default (b, bm, x, y, dx, dy, w, h, minterm, mask);
      return;
    }

  for (short i = 0; i < bm->Depth; i++)
    {
      UBYTE *src = bm->Planes[i] + y * bm->BytesPerRow + (x >> 3);
      UBYTE *dst = (UBYTE *) ((ULONG) b->MemoryBase + addr);
      for (short j = 0; j < h; j++)
	{
	  __builtin_memcpy (dst, src, line_size);
	  dst += line_size;
	  src += bm->BytesPerRow;
	}
      addr += plane_size;
    }
  regs[0] = dx;
  regs[1] = dy;
  regs[2] = mask | bm->Depth << 8;
}

/* The byte is tested in a data register or in memory.  */
/* { dg-final { scan-assembler-not {(tst|cmp)\.b [^\n]*a[0-7]\n} } } */
