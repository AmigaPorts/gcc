/* _Static_warning is _Static_assert that only warns.  */
/* { dg-do compile } */
_Static_warning (0, "deprecated"); /* { dg-warning "static warn-only assertion failed: \"deprecated\"" } */
_Static_warning (1, "not shown");
int f (void) { _Static_warning (sizeof (int) == 4, "int is 32 bits"); return 0; }
