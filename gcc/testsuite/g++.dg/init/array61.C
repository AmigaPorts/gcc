// PR c++/92385
// { dg-do compile { target c++11 } }
// { dg-additional-options -fdump-tree-gimple }
// Without symbol aliases the complete and base constructors are separate
// functions rather than one being an alias of the other, so the constructor
// name appears twice.  Darwin is the usual such target; m68k-amigaos is another.
// { dg-final { scan-tree-dump-times "item::item" 1 "gimple" { target alias } } }
// { dg-final { scan-tree-dump-times "item::item" 2 "gimple" { target { ! alias } } } }

struct item {
  int i;
  item();
};

struct item_array {
  item a[10];
  item_array();
};

item_array::item_array() : a{} {}
