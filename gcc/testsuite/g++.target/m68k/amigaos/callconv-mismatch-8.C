// Explicit and implicit default conversions are all quiet when disabled.

// { dg-do compile { target c++11 } }
// { dg-options "-m68000 -Os -mregparm=3 -Wno-callconv-mismatch" }

#include "callconv-mismatch-7.C"
