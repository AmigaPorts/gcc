/* The hunk format has no symbol visibility: a visibility the user writes
   is still diagnosed and ignored.  */

/* { dg-do compile } */
/* { dg-skip-if "amigaos only" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2" } */

int __attribute__ ((visibility ("hidden"))) hidden_var = 1; /* { dg-warning "visibility attribute not supported" } */

void __attribute__ ((visibility ("hidden")))
hidden_fn (void) /* { dg-warning "visibility attribute not supported" } */
{
}

/* { dg-final { scan-assembler-not "\\.hidden" } } */
