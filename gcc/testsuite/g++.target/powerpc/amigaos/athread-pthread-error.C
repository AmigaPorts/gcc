// Only clib4 has pthreads, and the driver says so rather than leave it
// to the linker.
// { dg-do link }
// { dg-skip-if "clib4 has pthreads" { amigaos_clib4 } }
// { dg-options "-athread=pthread" }
// { dg-error "athread=pthread requires -mcrt=clib4" "" { target *-*-* } 0 }

int main() { return 0; }
