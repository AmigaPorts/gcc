/* Slim LTO members of a static library must be pulled in when they are
   needed only by a later member of the same library.  The harness puts
   _1, _2 and _3 into the archive in that order: main needs f1 and f3,
   and f3 needs f2, so f2's member sits before the only member that
   references it.  A linker whose archive index does not carry the IR
   symbols has to probe each member through the plugin, and one that
   forgets the result of that probe on a later pass drops f2.  */
/* { dg-lto-do ar-link } */
/* { dg-lto-options { { -O2 -flto -fno-fat-lto-objects } } } */

extern int f1 (int);
extern int f3 (int);

int
main (int argc, char **argv)
{
  return f1 (argc) + f3 (argc);
}
