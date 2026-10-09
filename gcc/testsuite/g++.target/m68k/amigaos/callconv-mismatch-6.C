// The new user-defined conversion diagnostics obey the warning option.

// { dg-do compile { target c++11 } }
// { dg-options "-m68000 -Os -mregparm=3 -Wno-callconv-mismatch" }

#include "callconv-mismatch-5.C"
