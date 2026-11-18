/* { dg-do compile } */
/* { dg-options "-O2 -fbaserel32 -m68020" } */
/* An atomic insn holds its memory operand twice, and both must stay the
   same.  With a symbol plus offset in the address, the base-relative pass
   gave each copy a register of its own: an unrecognizable insn, with
   --enable-threads=amigaos, which has the atomic insns.  */

struct s { int a, b, c; char pad[52]; };
struct s v[4];

void lock (struct s *);

void
bump (unsigned i, int proxy)
{
  struct s *p = &v[i % 4];

  lock (p);
  if (proxy)
    __atomic_fetch_add (&p->c, 1, __ATOMIC_RELEASE);
  lock (p);
}
