// { dg-options "-m68000 -mregparm=3 -std=gnu++11" }

#include "amigaos-callconv-mismatch.H"

fnptr_t selected = choose (true);
fnptr_t instantiated = choose_template (true);
fnptr_t mismatched = stack_pointer {}; // { dg-warning "different calling convention" }

void
use_defaults ()
{
  cast_default ();
  cast_default ();
  moved_default ();
  template_default (1);
  mixed_defaults (); // { dg-warning "different calling convention" }
}
