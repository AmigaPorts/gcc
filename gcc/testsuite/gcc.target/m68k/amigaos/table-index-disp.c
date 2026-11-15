/* { dg-do compile } */
/* { dg-options "-O2 -m68020-60" } */
/* An entry of a constant table row, whose address is taken once and read
   twice: the row offset goes into a (bd,Xn.l*8) displacement, not into
   the index register with an add.  A register plus a symbol costs more
   than (d16,An), so the second load stays e[1] until combine can fold
   the row offset into the address.  */

extern unsigned int t[8][256][2];

void
f (const unsigned char *p, unsigned int *out)
{
  unsigned int h = 0, l = 0;
  const unsigned int *e;
  e = t[6][p[0]]; h |= e[0]; l |= e[1];
  e = t[5][p[1]]; h |= e[0]; l |= e[1];
  out[0] = h;
  out[1] = l;
}

/* Each row offset is in a displacement: of the loads, or of a lea of the
   row's address that the loads go through.  */
/* { dg-final { scan-assembler-not {add\.l #} } } */
/* { dg-final { scan-assembler {_t\+(12288|12292)\(d[0-7]\.l\*8\)} } } */
/* { dg-final { scan-assembler {_t\+(10240|10244)\(d[0-7]\.l\*8\)} } } */
