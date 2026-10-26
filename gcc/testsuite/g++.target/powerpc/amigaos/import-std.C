// import std.
// { dg-do run }
// { dg-additional-options "-O2 -std=c++23 -fmodules --compile-std-module" }

import std;
int main() { std::println("{}", 42); }
