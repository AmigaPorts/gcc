// Test that we don't need libsupc++ just for __cxa_pure_virtual.
// { dg-do link }
// { dg-require-weak "" }
// { dg-additional-options "-fno-rtti -nostdlib++" }
// { dg-additional-options "-Wl,-undefined,dynamic_lookup" { target *-*-darwin* } }
// { dg-xfail-if "AIX weak" { powerpc-ibm-aix* } }
// { dg-xfail-if PR119369 { amdgcn-*-* } }
// The amiga hunk linker does not resolve undefined weak symbols to 0.
// { dg-skip-if "No weak unsat" { m68k-*-amigaos* } }

struct A
{
  int i;
  virtual void f() = 0;
  A(): i(0) {}
};

struct B: A
{
  virtual void f() {}
};

int main()
{
  B b;
}
