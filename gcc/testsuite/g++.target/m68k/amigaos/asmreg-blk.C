// Amiga register-parameter ABI: a struct with no integer mode cannot be
// bound to a register, in the definition and in a call.

// { dg-do compile }
// { dg-options "-O2 -m68020" }

struct S3 { int a, b, c; };

extern int e (S3 s __asm ("d0"));

int
f (S3 s __asm ("d0")) // { dg-error "register .d0. cannot hold" }
{
  return s.a;
}

int
g (S3 *p)
{
  return e (*p); // { dg-error "register .d0. cannot hold" }
}
