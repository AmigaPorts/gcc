// { dg-do run { target c++11 } }
// { dg-require-effective-target tls_runtime }
// { dg-add-options tls }
// { dg-additional-sources "tls-extern-init-aux.cc" }
// A thread_local with a dynamic initializer, defined in another file, is
// initialized on its first use here.  That takes an alias and a weak
// reference to its init function: without aliases, x was read as 0.

extern thread_local int x;
extern thread_local int y;

int f ()
{
  return 42;
}

int main ()
{
  // y has no init function: the weak reference to it is null
  return x != 42 || y != 7;
}
