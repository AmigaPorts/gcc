// { dg-do assemble { target fpic } }
// { dg-options "-fexceptions -fPIC -S" }

int
main() { throw 1; }
