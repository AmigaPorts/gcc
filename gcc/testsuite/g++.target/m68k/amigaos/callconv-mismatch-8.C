// Explicit and implicit default conversions are all quiet when disabled.

// { dg-do compile { target c++11 } }
// { dg-options "-m68000 -Os -mregparm=3 -Wno-callconv-mismatch" }
// { dg-skip-if "-m68000 has no 32-bit base-relative data" { *-*-* } { "-fbaserel32" "-resident32" } }

#include "callconv-mismatch-7.C"
