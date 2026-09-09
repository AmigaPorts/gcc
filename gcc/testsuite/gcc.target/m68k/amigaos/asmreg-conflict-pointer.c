/* Amiga register-parameter ABI: a function binding its parameters to
   registers must not convert to a pointer to a function that takes them
   on the stack, or to one binding other registers; a call through that
   pointer would pass the arguments where the callee does not look.  A
   pointer type with the same bindings is fine.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */

long f (long a __asm ("d0"), long b __asm ("d1"));

typedef long (*plain_fn) (long a, long b);
typedef long (*other_fn) (long a __asm ("d0"), long b __asm ("a0"));
typedef long (*same_fn) (long a __asm ("d0"), long b __asm ("d1"));

plain_fn p1 = f;	/* { dg-error "incompatible pointer type" } */
other_fn p2 = f;	/* { dg-error "incompatible pointer type" } */
same_fn p3 = f;		/* { dg-bogus "incompatible" } */

long call (same_fn fn)
{
  return fn (1, 2);
}
