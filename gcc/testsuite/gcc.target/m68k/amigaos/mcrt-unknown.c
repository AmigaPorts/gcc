/* An -mcrt= name the driver doesn't know is an error, not a link without
   any runtime, whichever board flags it arrives with.  */

/* { dg-do compile } */
/* { dg-additional-options "-mcrt=libnix" } */
/* { dg-error "unknown C runtime 'libnix'" "" { target *-*-* } 0 } */
/* { dg-prune-output "compilation terminated" } */

int
main (void)
{
  return 0;
}
