/* { dg-do compile } */
/* { dg-options "-Os -fbaserel -m68020-60" } */
/* Reduced from g++.dg/torture/pr84132.C.  The store's address is a scaled
   index plus a register plus "symbol + 2", and the baserel pass turns the
   symbol into a register too: three registers, which it must bring down to
   base, index and displacement rather than leave an address no insn
   matches.  */

struct g {
    char b;
    unsigned char *d[];
} e;
unsigned char f;

void i (void)
{
  for (int h;; h++)
    {
      unsigned a = h;
      for (int c = 0; c < 256; c += 6)
	for (int h = c; h < c + 6; h++)
	  e.d[h + a] = &f;
    }
}
