// PR c++/120125
// { dg-additional-options "-fmodules -fdeclone-ctor-dtor" }

import M;

int main() {
  __shared_ptr<int> s1;
  __shared_ptr<double> s2;
}

// Without aliases the importer keeps local copies of the unified [cd]tor and
// its thunk: harmless, as they are local symbols.
// { dg-final { scan-assembler-not {_?_ZNW1M12__shared_ptrIiEC[1-4]Ev:} { xfail m68k-*-amigaos* } } }
// { dg-final { scan-assembler {_?_ZNW1M12__shared_ptrIdEC2Ev:} { target alias } } }
// { dg-final { scan-assembler {_?_ZNW1M12__shared_ptrIdEC1Ev:} { target { ! alias } } } }
