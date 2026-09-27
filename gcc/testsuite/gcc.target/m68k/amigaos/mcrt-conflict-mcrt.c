/* Two -mcrt= selectors that disagree are an error too, whichever board
   flags they arrive with.  */

/* { dg-do compile } */
/* { dg-additional-options "-mcrt=newlib -mcrt=nix13" } */
/* { dg-error "both select a C runtime" "" { target *-*-* } 0 } */
/* { dg-prune-output "compilation terminated" } */

int
main (void)
{
  return 0;
}
