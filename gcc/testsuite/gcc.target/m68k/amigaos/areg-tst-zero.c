/* { dg-do compile } */
/* { dg-options "-Os -m68000 -fomit-frame-pointer" } */
/* The 68000 has no tst.l An, and cmpa.w #0,An is 4 bytes.  With a free
   data register, move.l An,Dn sets the condition codes in 2.  */

struct node {
  struct node *next;
  int val;
};

int
walk (struct node *p)
{
  if (!p)
    return -1;
  struct node *n = p->next;
  if (!n)
    return p->val;
  return n->val;
}

int
nonnull (void *p)
{
  __asm__ volatile ("" : "+a" (p));
  return p != 0;
}

/* { dg-final { scan-assembler-not {cmp\.w #0,a} } } */
/* { dg-final { scan-assembler {move\.l a[0-7],d[0-7]\n\tj(eq|ne) } } } */
/* { dg-final { scan-assembler {move\.l a[0-7],d[0-7]\n\tsne d[0-7]} } } */
