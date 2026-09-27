/* A program gets one C runtime, so the obsolete -noixemul next to an
   explicit -mcrt= is an error rather than a silent choice: the specs would
   otherwise hand the linker two startup files.  */

/* { dg-do compile } */
/* { dg-additional-options "-noixemul -mcrt=newlib" } */
/* { dg-error "both select a C runtime" "" { target *-*-* } 0 } */
/* { dg-prune-output "compilation terminated" } */

int
main (void)
{
  return 0;
}
