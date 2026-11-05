/* { dg-do compile } */
/* { dg-options "-Os -m68000 -fdump-tree-sink1" } */

/* Multiplication of a runtime element size needs a libcall on the 68000.
   PRE hoists it out of the copy block.  Sinking it back immediately before
   d0's assignment clobbers the a0/a1 arguments already assigned there.
   Keep the sunk expression before the hard-register assignments.  */

struct Entry { char *a; };
struct Vec
{
  void *pool;
  char *items;
  long count, cap, esize;
};

extern void *SysBase;
extern void *alloc (void *, unsigned long);

/* The register operands of an AmigaOS CopyMem call.  */
static inline void
copy_mem (void *source, void *dest, unsigned long size)
{
  register void *a6 __asm ("a6") = SysBase;
  register void *a0 __asm ("a0") = source;
  register void *a1 __asm ("a1") = dest;
  register unsigned long d0 __asm ("d0") = size;
  __asm volatile ("jsr a6@(-624:W)"
                  : "+a" (a0), "+a" (a1), "+d" (d0)
                  : "a" (a6)
                  : "d1", "cc", "memory");
}

struct Entry *
add (struct Vec *v, char *s)
{
  struct Entry *e;
  if (v->count == v->cap)
    {
      long ncap = v->cap ? v->cap * 2 : 64;
      char *n = alloc (v->pool, ncap * v->esize);
      if (!n)
        return 0;
      if (v->items)
        copy_mem (v->items, n, v->count * v->esize);
      v->items = n;
      v->cap = ncap;
    }
  e = (struct Entry *) (v->items + v->count * v->esize);
  e->a = s;
  v->count++;
  return e;
}

/* The multiply must not be sunk between a1 and d0.  */
/* { dg-final { scan-tree-dump-not {a1 = [^\n]*;\n  [^\n]* \* [^\n]*;} "sink1" } } */
/* { dg-final { scan-assembler "__mulsi3" } } */
