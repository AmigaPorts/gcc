/* { dg-do compile } */
/* { dg-options "-O2 -m68000 -fomit-frame-pointer" } */
/* As block-loop-frame-size.c, for memcpy and memset loops.  Arguments that
   are being pushed can't be popped before the loop, so a large structure
   passed by value has to be copied by memcpy instead.  */

register void *base __asm ("a5");

struct big { long a[75]; };

void use (void *);
void take (int, struct big);

void
copy_after_call (void *p, const void *q)
{
  char buf[36];

  use (p);
  __builtin_memcpy (buf, q, sizeof buf);
  use (buf);
}

void
clear_after_call (void *p)
{
  char buf[300];

  use (p);
  __builtin_memset (buf, 0, sizeof buf);
  use (buf);
}

void
pass_by_value (void *p, const struct big *q)
{
  use (p);
  take (1, *q);
}

/* { dg-final { scan-assembler "dbra" } } */
/* { dg-final { scan-assembler "_memcpy" } } */
/* { dg-final { scan-assembler-not "link" } } */
