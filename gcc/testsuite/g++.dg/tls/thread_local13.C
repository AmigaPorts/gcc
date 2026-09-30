// PR c++/109164
// { dg-do run { target c++11 } }
// { dg-options "-O2" }
// { dg-add-options tls }
// { dg-require-effective-target tls_runtime }
// Without symbol aliases the default is -fno-extern-tls-init, and a
// thread_local defined in another TU is not initialized before its use here.
// { dg-require-alias "" }
// { dg-additional-sources "thread_local13-aux.cc" }

struct S { virtual void foo (); int s; };
extern thread_local S &t;
bool bar ();

bool
baz ()
{
  while (1)
    {
      t.foo ();
      if (!bar ())
        return false;
    }
}
