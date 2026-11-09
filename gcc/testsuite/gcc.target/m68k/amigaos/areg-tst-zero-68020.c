/* { dg-do compile } */
/* { dg-options "-Os -m68020 -fomit-frame-pointer" } */
/* The 68020 has tst.l An: no copy to a data register.  */

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

/* { dg-final { scan-assembler-times {tst\.l a[0-7]\n} 3 } } */
/* { dg-final { scan-assembler-not {move\.l a[0-7],d[0-7]\n\t(j|s)(eq|ne) } } } */
/* { dg-final { scan-assembler-not {cmp\.w #0,a} } } */
