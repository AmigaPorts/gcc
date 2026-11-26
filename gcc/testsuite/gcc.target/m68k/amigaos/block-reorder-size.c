/* { dg-do compile } */
/* { dg-options "-Os -fdump-rtl-bbro" } */

extern void left (void);
extern void right (void);
extern void finish (void);

void
choose (int x)
{
  if (x)
    left ();
  else
    right ();
  finish ();
}

/* { dg-final { scan-rtl-dump {Reordering with the STC algorithm} "bbro" } } */
