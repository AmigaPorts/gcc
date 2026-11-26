/* A register-argument test that returns early needs no prologue on that
   path.  */
/* { dg-do compile } */
/* { dg-options "-O2 -fdump-rtl-pro_and_epilogue" } */

extern void use (unsigned *, unsigned *, unsigned *, unsigned *);

unsigned
f (unsigned *p __asm ("a0"), unsigned x __asm ("d0"))
{
  if (x & 255)
    return x + 1;
  unsigned a = p[0], b = p[1], c = p[2], d = p[3];
  use (&a, &b, &c, &d);
  use (&d, &c, &b, &a);
  return a + b + c + d + x;
}

/* { dg-final { scan-rtl-dump "Performing shrink-wrapping" "pro_and_epilogue" } } */
