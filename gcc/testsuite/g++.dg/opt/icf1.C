// Test that -fipa-icf combines i and j.
// { dg-do run { target c++11 } }
// { dg-options -fipa-icf }
// ICF merges variables by making one an alias of the other.
// { dg-require-alias "" }

[[no_unique_address]] extern const int i[] = { 1,2,3 };
[[no_unique_address]] extern const int j[] = { 1,2,3 };

[[gnu::noipa]] void f (const void *a, const void *b)
{
  if (a != b) __builtin_abort();
}

int main()
{
  f (&i, &j);
}
