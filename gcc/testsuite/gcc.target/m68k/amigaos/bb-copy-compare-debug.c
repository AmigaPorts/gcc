/* The size bound on block duplication in bb-reorder, which m68k needs
   because it has no length attribute, counted debug insns as code, so
   with -g a block could fall over the limit that it stayed under
   without, and the code differed.  A C rendering of
   g++.dg/opt/pr100148.C, which caught it.  */
/* { dg-do compile } */
/* { dg-options "-O2 -fno-dce -fno-tree-dce -fno-tree-dominator-opts -fno-tree-sink -fcompare-debug" } */

int i;
enum E { E0 } e, ee;
_Bool a, b;

_Bool
baz (int x)
{
  return ee;
}

_Bool bar (void);

void
foo (void)
{
  switch (ee)
    {
    case 0:
      e = (enum E) (a ? a : i);
    case 1:
      !(b || (baz (0) && bar ()));
    }
}
