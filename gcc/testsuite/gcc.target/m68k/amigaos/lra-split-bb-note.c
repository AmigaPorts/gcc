/* Reduced from newlib dtoa.c.  y lives in d0 and is live across the call
   in the cold block, so LRA splits it there and anchors the save on the
   basic block note; the sp offset fixup for reloads emitted after an insn
   must not ask a note for its recog data.  */
/* { dg-do compile } */
/* { dg-options "-O2 -mlra -m68000" } */

int g (int);
void h (void);

int
f (int x, int k)
{
  int y = g (x);
  if (__builtin_expect (k, 0))
    {
      h ();
      y += k;
    }
  return y;
}
