/* { dg-do compile } */
/* { dg-options "-O2 -fbaserel -m68020 -fomit-frame-pointer" } */
/* Reduced from libpng's pngunknown.c.  The loop's trip count was derived
   from a pointer difference and folded into "(const (minus N sym))",
   printed as "#-26-(_chunk_info:W)", which the assembler rejects: no
   relocation subtracts a symbol.  The constant must be split instead. */

struct info { char name[5]; unsigned flag; unsigned tag; int unknown;
	      int all; int position; int keep; };
static struct info chunk_info[25];
extern int fseek (void *, long, int);

int perform (void *fp, long off, int which)
{
  int i = 25;
  fseek (fp, off, 0);
  while (--i >= 0)
    chunk_info[i].keep = 0;
  return chunk_info[which].flag;
}

/* { dg-final { scan-assembler-not {:W\)} } } */
/* { dg-final { scan-assembler-not {#-[0-9]+-_} } } */
