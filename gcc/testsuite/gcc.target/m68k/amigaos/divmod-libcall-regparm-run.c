/* divmod-libcall-run.c with register arguments: the divmod libcalls still
   take their operands on the stack, as the helpers expect.  */

/* { dg-do run } */
/* { dg-skip-if "the 68000 has no -fbaserel32" { *-*-* } { "-fbaserel32" "-resident32" } { "" } } */
/* { dg-options "-m68000 -O2 -mregparm=4 -mmangle-regparm" } */

#include "divmod-libcall-run.c"
