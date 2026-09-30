/* PR c/51628.  */
/* { dg-do compile } */
/* { dg-options "-O" } */
/* int is 2-byte aligned on m68k, so a member of a packed struct aligned to
   2 is not misaligned and there is nothing to warn about.  */
/* { dg-skip-if "int alignment is 2" { m68k*-*-* } { "*" } { "-malign-int" } } */

struct pair_t
{
  int x;
  int i;
} __attribute__((packed, aligned (2)));

extern struct pair_t p;
extern int *x;
extern void bar (int *);

int *addr = &p.i;
/* { dg-warning "may result in an unaligned pointer value" "" { target { ! default_packed } } .-1 } */

int *
foo (void)
{
  struct pair_t arr[2] = { { 1, 10 }, { 2, 20 } };
  int *p0, *p1;
  p0 = &arr[0].i;
/* { dg-warning "may result in an unaligned pointer value" "" { target { ! default_packed } } .-1 } */
  bar (p0);
  p1 = &arr[1].i;
/* { dg-warning "may result in an unaligned pointer value" "" { target { ! default_packed } } .-1 } */
  bar (p1);
  bar (&p.i);
/* { dg-warning "may result in an unaligned pointer value" "" { target { ! default_packed } } .-1 } */
  x = &p.i;
/* { dg-warning "may result in an unaligned pointer value" "" { target { ! default_packed } } .-1 } */
  return &p.i;
/* { dg-warning "may result in an unaligned pointer value" "" { target { ! default_packed } } .-1 } */
}
