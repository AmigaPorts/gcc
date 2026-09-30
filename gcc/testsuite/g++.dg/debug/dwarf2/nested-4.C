// PR debug/53235
// { dg-options "-gdwarf-4 -fdebug-types-section" }
// Type units need COMDAT groups, which the hunk format does not have, so
// -fdebug-types-section emits none.
// { dg-skip-if "no COMDAT groups" { m68k-*-amigaos* } }
// { dg-final { scan-assembler-times "debug_types" 2 { xfail { powerpc-ibm-aix* || { *-*-darwin* || { *-*-solaris2.* && { comdat_group && { ! gas } } } } } } } }

namespace E {
  class O {};
  void f (O o) {}
}
namespace F {
  class O {};
  void f (O fo) {}
}
E::O eo;
int main () {}
