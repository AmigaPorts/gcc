/* { dg-do compile } */
/* { dg-options "-O2 -m68000 -fomit-frame-pointer" } */

/* The scan keeps its dbra at -O2.  A 16-bit counter would be an IV of
   its own beside i: IVOPTs then counts with i and the loop loses the
   dbra (112 bytes to 144).  */

struct ent { unsigned long valid; unsigned long last; char pad[44]; };
struct tab { struct ent e[16]; unsigned long size; };

int
evict (struct tab *t)
{
  unsigned i, oldest = 16;
  for (i = 0; i < 16; i++)
    {
      if (!t->e[i].valid)
	continue;
      if (oldest == 16 || (long) (t->e[i].last - t->e[oldest].last) < 0)
	oldest = i;
    }
  if (oldest == 16)
    return -1;
  t->e[oldest].valid = 0;
  t->size--;
  return (int) oldest;
}

/* { dg-final { scan-assembler {\tdbra d[0-7],} } } */
